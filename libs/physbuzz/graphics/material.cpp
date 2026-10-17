#include "material.hpp"

#include "../app/application.hpp"
#include "layout.hpp"
#include "model.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

MaterialAllocator::MaterialAllocator() {}

bool MaterialAllocator::Resources::build() {
    std::optional<Resource<DescriptorLayout>> layoutRef = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/material",
        {{
            .bindings = {
                {
                    // textures
                    .type = DescriptorLayout::Type::eCombinedImageSampler,
                    .flags = DescriptorLayout::DescriptorBindingFlags::ePartiallyBound | DescriptorLayout::DescriptorBindingFlags::eUpdateAfterBind,
                    .count = 512,
                },
            },
            .flags = DescriptorLayout::Flags::eUpdateAfterBindPool,
            .lifetime = DescriptorLayout::Lifetime::Global,
        }});

    if (!layoutRef) {
        return false;
    }

    layout = std::move(*layoutRef);

    return true;
}

bool MaterialAllocator::build() {
    bool success = true;

    success &= m_Resources.build();

    m_MaterialBuffer.build(sizeof(MaterialBuffer));

    m_Events.build = ResourceRegistry<Material>::Events.addCallback<OnResourceBuild>([&](const OnResourceBuild &event) {
        Resource<Material> material = {event.id};

        for (const auto &[type, texture] : material->textures) {
            // new texture loaded into table, map to bindless descriptor
            Image::ViewInfo viewInfo = {
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = Image::RemainingMipLevels,
                    .layerCount = 1,
                },
            };

            if (!texture->getData().image.getData().views.contains(viewInfo)) {
                Logger::ERROR("[MaterialAllocator] Required view not found in texture.");
                continue;
            }

            // texture already loaded
            if (!m_Textures.add(texture)) {
                continue;
            }

            App::LayoutAllocator.write(m_Resources.layout, texture, viewInfo, 0, m_Textures.query(texture));
        }

        if (m_Materials.add(material)) {
            update(material);
        }
    });

    m_Events.destroy = ResourceRegistry<Material>::Events.addCallback<OnResourceDestroy>([&](const OnResourceDestroy &event) {
        Resource<Material> material = {event.id};

        // untrack from material table
        m_Materials.remove(material);

        // if no other material references its textures, untrack it too
        for (const auto &[type, texture] : material->textures) {
            bool shouldErase = true;

            for (const auto &[materialId, _] : m_Materials.getResources()) {
                for (const auto &[_, texture2] : Resource<Material>(materialId)->textures) {
                    if (texture2 == texture) {
                        shouldErase = false;
                    }
                }
            }

            if (shouldErase) {
                m_Textures.remove(texture);
            }
        }
    });

    return success;
}

bool MaterialAllocator::destroy() {
    m_MaterialBuffer.destroy();
    m_Materials.clear();
    m_Textures.clear();
    m_Buffer.clear();
    m_BufferIsDirty = false;

    ResourceRegistry<Material>::Events.eraseCallback<OnResourceBuild>(m_Events.build);
    ResourceRegistry<Material>::Events.eraseCallback<OnResourceDestroy>(m_Events.destroy);

    return true;
}

void MaterialAllocator::update(Resource<Material> material) {
    // create a new material buffer
    MaterialBuffer buffer;

    for (const auto &[type, texture] : material->textures) {
        switch (type) {
        case TextureType::Albedo:
            buffer.albedoTextureId = m_Textures.query(texture);
            break;

        case TextureType::Normal:
            buffer.normalTextureId = m_Textures.query(texture);
            break;

        case TextureType::Metallic:
            buffer.metallicTextureId = m_Textures.query(texture);
            break;

        case TextureType::Roughness:
            buffer.roughnessTextureId = m_Textures.query(texture);
            break;

        case TextureType::Emission:
            buffer.emissionTextureId = m_Textures.query(texture);
            break;

        case TextureType::AmbientOcclusion:
            buffer.ambientOcclusionTextureId = m_Textures.query(texture);
            break;

        case TextureType::Unknown:
            continue;
        }
    }

    buffer.albedoFactor = material->albedoFactor;
    buffer.metallicFactor = material->metallicFactor;
    buffer.roughnessFactor = material->roughnessFactor;

    std::uint32_t idx = m_Materials.query(material);

    if (m_Buffer.size() <= idx) {
        m_Buffer.resize(idx + 1);
    }

    m_Buffer[idx] = buffer;
    m_BufferIsDirty = true;
}

void MaterialAllocator::refresh(const RenderContext &context) {
    ZoneScopedN("Material/Refresh");

    if (m_BufferIsDirty) {
        // TODO a buffer copy + offset write would be better
        m_MaterialBuffer.rebuild(*context.deletionQueue, m_Buffer.size() * sizeof(MaterialBuffer));
        m_MaterialBuffer.map<MaterialBuffer>(context.command, context.deletionQueue, m_Buffer, 0);

        m_BufferIsDirty = false;
    }
}

std::uint32_t MaterialAllocator::query(const Resource<Material> &material) const {
    return m_Materials.query(material);
}

std::uint32_t MaterialAllocator::query(const Resource<Texture> &texture) const {
    return m_Textures.query(texture);
}

const Buffer &MaterialAllocator::getMaterialBuffer() const {
    return m_MaterialBuffer;
}

const MaterialAllocator::Resources &MaterialAllocator::getResources() const {
    return m_Resources;
}

} // namespace Physbuzz
