#pragma once

#include "../resources/table.hpp"
#include "memory.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

struct RenderContext;
class DescriptorLayout;
class Texture;

enum class TextureType;

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

    struct Resources {
        Resource<DescriptorLayout> layout;

        bool build();
    };

    MaterialAllocator();

    bool build();
    bool destroy();

    void refresh(const RenderContext &context);
    void update(Resource<Material> material);

    std::uint32_t query(const Resource<Material> &material) const;
    std::uint32_t query(const Resource<Texture> &texture) const;

    const Buffer &getMaterialBuffer() const;
    const Resources &getResources() const;

  private:
    ResourceTable<Material> m_Materials;
    ResourceTable<Texture> m_Textures;

    std::vector<MaterialBuffer> m_Buffer;
    bool m_BufferIsDirty;

    Buffer m_MaterialBuffer = {{}};
    Resources m_Resources;

    struct {
        EventID build = -1;
        EventID destroy = -1;
    } m_Events;
};

} // namespace Physbuzz
