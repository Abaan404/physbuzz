#include "material.hpp"

#include "../app/application.hpp"
#include "layout.hpp"
#include "model.hpp"

namespace Physbuzz {

namespace Builtin {

bool LayoutMaterial::build() {
    if (ResourceRegistry<PipelineLayout>::contains(Resource)) {
        return true;
    }

    bool success = true;

    success &= ResourceRegistry<PipelineLayout>::insert(
        Resource,
        {{
            .bindings = {
                {
                    // textures
                    .type = PipelineLayout::Type::eCombinedImageSampler,
                    .flags = PipelineLayout::BindingFlagBits::ePartiallyBound | PipelineLayout::BindingFlagBits::eUpdateAfterBind,
                    .count = 512,
                },
            },
            .flags = PipelineLayout::Flags::eUpdateAfterBindPool,
            .lifetime = PipelineLayout::Lifetime::Global,
        }});

    return success;
}

} // namespace Builtin

MaterialAllocator::MaterialAllocator(const Info &info)
    : m_Info(info) {};

bool MaterialAllocator::build() {
    bool success = true;

    if (m_Info.layout == Builtin::LayoutMaterial::Resource) {
        success &= Builtin::LayoutMaterial::build();
    }

    m_MaterialBuffer.build(sizeof(Builtin::LayoutMaterial::MaterialBuffer));

    m_Events.build = ResourceRegistry<Material>::Events.addCallback<OnResourceBuild>([&](const OnResourceBuild &event) {
        Resource<Material> material = {event.identifier};

        for (const auto &[type, texture] : material->textures) {
            // new texture loaded into table, map to bindless descriptor
            if (m_Textures.add(texture)) {
                App::LayoutAllocator.write(m_Info.layout, texture, 0, m_Textures.query(texture));
            }
        }

        if (m_Materials.add(material)) {
            // create a new material buffer
            Builtin::LayoutMaterial::MaterialBuffer buffer = {
                .diffuseTextureId = material->textures.contains(TextureType::Diffuse)
                                        ? m_Textures.query(material->textures.at(TextureType::Diffuse))
                                        : -1,
                .specularTextureId = material->textures.contains(TextureType::Specular)
                                         ? m_Textures.query(material->textures.at(TextureType::Specular))
                                         : -1,
                .heightTextureId = material->textures.contains(TextureType::Height)
                                         ? m_Textures.query(material->textures.at(TextureType::Height))
                                         : -1,
                .specularity = material->shininess,
            };

            std::uint32_t idx = m_Materials.query(material);

            if (m_Buffer.size() <= idx) {
                m_Buffer.resize(idx + 1);
            }

            m_Buffer[idx] = buffer;
            m_BufferIsDirty = true;
        }
    });

    m_Events.destroy = ResourceRegistry<Material>::Events.addCallback<OnResourceDestroy>([&](const OnResourceDestroy &event) {
        Resource<Material> material = {event.identifier};

        // untrack from material table
        m_Materials.remove(material);

        // if no other material references its textures, untrack it too
        for (const auto &[type, texture] : material->textures) {
            bool shouldErase = true;

            for (const auto &[material2, _] : m_Materials.getResources()) {
                for (const auto &[_, texture2] : material2->textures) {
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

void MaterialAllocator::refresh(const RenderContext &context) {
    if (m_BufferIsDirty) {
        // TODO a buffer copy + offset write would be better
        m_MaterialBuffer.rebuild(context, m_Buffer.size() * sizeof(Builtin::LayoutMaterial::MaterialBuffer));
        m_MaterialBuffer.update<Builtin::LayoutMaterial::MaterialBuffer>(context, m_Buffer, 0);

        m_BufferIsDirty = false;
    }
}

std::uint32_t MaterialAllocator::query(const Resource<Material> &material) const {
    return m_Materials.query(material);
}

std::uint32_t MaterialAllocator::query(const Resource<Texture> &texture) const {
    return m_Textures.query(texture);
}

const StaticBuffer &MaterialAllocator::getMaterialBuffer() const {
    return m_MaterialBuffer;
}

} // namespace Physbuzz
