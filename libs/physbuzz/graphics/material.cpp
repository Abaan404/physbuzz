#include "material.hpp"

#include "../render/model.hpp"
#include "layout.hpp"

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
                    .count = 32,
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

    return success;
}

bool MaterialAllocator::destroy() {
    m_MaterialBuffer.destroy();
    m_Materials.clear();
    m_Textures.clear();

    return true;
}

void MaterialAllocator::refresh(const Model &model, const RenderContext &context) {
    for (const auto &mesh : model.getInfo().meshes) {
        for (const auto &[type, texture] : mesh.material->textures) {
            // new texture loaded into table, map to bindless descriptor
            if (m_Textures.add(texture)) {
                context.systems.allocator->write(m_Info.layout, texture, 0, m_Textures.query(texture));
            }
        }

        // fetch material ids
        if (m_Materials.add(mesh.material)) {
            // create a new material buffer
            Builtin::LayoutMaterial::MaterialBuffer material = {
                .diffuseTextureId = mesh.material->textures.contains(TextureType::Diffuse)
                                        ? m_Textures.query(mesh.material->textures.at(TextureType::Diffuse))
                                        : 0,
                .specularTextureId = mesh.material->textures.contains(TextureType::Specular)
                                         ? m_Textures.query(mesh.material->textures.at(TextureType::Specular))
                                         : 0,
                .specularity = mesh.material->shininess,
            };

            // upload material data
            m_MaterialBuffer.update<Builtin::LayoutMaterial::MaterialBuffer>(context, {material}, m_Materials.query(mesh.material));
        }
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
