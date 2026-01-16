#pragma once

#include "../../resources/table.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

class Model;
class Texture;
class Material;
class DynamicBuffer;
class StaticBuffer;
class PipelineLayout;

struct RenderContext;

namespace Builtin {

namespace RenderLayoutGlobal {

struct MaterialBuffer {
    std::uint32_t diffuseTextureId;
    std::uint32_t specularTextureId;
    float specularity;
};

inline Resource<StaticBuffer> ResourceBufferMaterials = {"builtin/materials"};
inline ResourceTable<Material> TableMaterial;

inline Resource<DynamicBuffer> ResourceBufferTextures = {"builtin/textures"};
inline ResourceTable<Texture> TableTexture;

inline Resource<PipelineLayout> Resource = {"builtin/global"};

bool build();

void refresh(const RenderContext &context, const Model &model);

} // namespace RenderLayoutGlobal

} // namespace Builtin

} // namespace Physbuzz
