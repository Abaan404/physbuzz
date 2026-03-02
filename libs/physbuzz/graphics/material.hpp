#pragma once

#include "../resources/table.hpp"
#include "descriptors/static.hpp"

namespace Physbuzz {

class PipelineLayout;
class Texture;
class Model;

enum class TextureType;

namespace Builtin {

namespace LayoutMaterial {

struct MaterialBuffer {
    std::uint32_t diffuseTextureId;
    std::uint32_t specularTextureId;
    std::uint32_t heightTextureId;
    float specularity;
};

inline Resource<PipelineLayout> Resource = {"builtin/material"};

bool build();

} // namespace LayoutMaterial

} // namespace Builtin

class Material {
  public:
    float shininess = 32.0f;
    std::unordered_map<TextureType, Resource<Texture>> textures;
};

template <>
struct IsResource<Material> : std::true_type {};

class MaterialAllocator {
  public:
    struct Info {
        Resource<PipelineLayout> layout = Builtin::LayoutMaterial::Resource;
    };

    MaterialAllocator(const Info &info);

    bool build();
    bool destroy();

    void refresh(const RenderContext &context);

    std::uint32_t query(const Resource<Material> &material) const;
    std::uint32_t query(const Resource<Texture> &texture) const;

    const StaticBuffer &getMaterialBuffer() const;

  private:
    Info m_Info;

    ResourceTable<Material> m_Materials;
    ResourceTable<Texture> m_Textures;

    std::vector<Builtin::LayoutMaterial::MaterialBuffer> m_Buffer;
    bool m_BufferIsDirty;

    StaticBuffer m_MaterialBuffer;

    struct {
        EventID build = -1;
        EventID destroy = -1;
    } m_Events;
};

} // namespace Physbuzz
