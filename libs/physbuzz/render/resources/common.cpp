#include "common.hpp"

#include "../../graphics/descriptors/static.hpp"
#include "../../graphics/layout.hpp"
#include "../model.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderLayoutGlobal::build() {
    if (ResourceRegistry<PipelineLayout>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<StaticBuffer>::contains(ResourceBufferMaterials)) {
        success &= ResourceRegistry<StaticBuffer>::insert(
            ResourceBufferMaterials,
            {},
            sizeof(MaterialBuffer));
    }

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

void RenderLayoutGlobal::refresh(const RenderContext &context, const Model &model) {
    Model::Info modelInfo = model.getInfo();

    for (const auto &mesh : modelInfo.meshes) {
        for (const auto &[type, texture] : mesh.material->textures) {
            // new texture loaded into table, map to bindless descriptor
            if (TableTexture.add(texture)) {
                context.systems.allocator->attach(
                    Builtin::RenderLayoutGlobal::Resource,
                    texture,
                    0, TableTexture.query(texture));
            }
        }

        // fetch material ids
        if (TableMaterial.add(mesh.material)) {
            // create a new material buffer
            Builtin::RenderLayoutGlobal::MaterialBuffer material = {
                .diffuseTextureId = mesh.material->textures.contains(TextureType::Diffuse)
                                        ? TableTexture.query(mesh.material->textures.at(TextureType::Diffuse))
                                        : 0,
                .specularTextureId = mesh.material->textures.contains(TextureType::Specular)
                                         ? TableTexture.query(mesh.material->textures.at(TextureType::Specular))
                                         : 0,
                .specularity = mesh.material->shininess,
            };

            std::size_t requiredMaterialSize = TableMaterial.size() * sizeof(Builtin::RenderLayoutGlobal::MaterialBuffer);
            if (Builtin::RenderLayoutGlobal::ResourceBufferMaterials->getSize() < requiredMaterialSize) {
                Builtin::RenderLayoutGlobal::ResourceBufferMaterials->resize(context, requiredMaterialSize);
            }

            // upload material data
            Builtin::RenderLayoutGlobal::ResourceBufferMaterials->update<Builtin::RenderLayoutGlobal::MaterialBuffer>(
                context,
                {material}, TableMaterial.query(mesh.material));
        }
    }
}

} // namespace Builtin

} // namespace Physbuzz
