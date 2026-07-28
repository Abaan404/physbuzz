#pragma once

#include "../resources/table.hpp"
#include "descriptors/static.hpp"

namespace Physbuzz {

class DescriptorLayout;
class Texture;
class Model;

enum class TextureType;

namespace Builtin {

namespace LayoutMaterial {

struct MaterialBuffer {
    std::uint32_t albedoTextureId = -1;
    std::uint32_t normalTextureId = -1;
    std::uint32_t metallicTextureId = -1;
    std::uint32_t roughnessTextureId = -1;
    std::uint32_t emissionTextureId = -1;
    std::uint32_t ambientOcclusionTextureId = -1;

    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec3 albedoFactor = glm::vec3(1.0f);
};

inline Resource<DescriptorLayout> Resource = {"builtin/material"};

bool build();

} // namespace LayoutMaterial

} // namespace Builtin

struct Material {
    std::unordered_map<TextureType, Resource<Texture>> textures;

    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec3 albedoFactor = glm::vec3(1.0f);
};

template <>
struct IsResource<Material> : std::true_type {};

class MaterialAllocator {
  public:
    MaterialAllocator();

    bool build();
    bool destroy();

    void refresh(const RenderContext &context);
    void update(Resource<Material> material);

    std::uint32_t query(const Resource<Material> &material) const;
    std::uint32_t query(const Resource<Texture> &texture) const;

    const StaticBuffer &getMaterialBuffer() const;

  private:
    ResourceTable<Material> m_Materials;
    ResourceTable<Texture> m_Textures;

    std::vector<Builtin::LayoutMaterial::MaterialBuffer> m_Buffer;
    bool m_BufferIsDirty;

    StaticBuffer m_MaterialBuffer = {{}};

    struct {
        EventID build = -1;
        EventID destroy = -1;
    } m_Events;
};

} // namespace Physbuzz
