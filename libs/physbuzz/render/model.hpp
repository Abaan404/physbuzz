#pragma once

#include "../resources/resources.hpp"
#include "mesh.hpp"
#include <assimp/scene.h>
#include <filesystem>

namespace Physbuzz {

class Texture;

enum class TextureType : std::uint32_t {
    None = aiTextureType_NONE,
    Diffuse = aiTextureType_DIFFUSE,
    Specular = aiTextureType_SPECULAR,
    Ambient = aiTextureType_AMBIENT,
    Emissive = aiTextureType_EMISSIVE,
    Height = aiTextureType_HEIGHT,
    Normals = aiTextureType_NORMALS,
    Shininess = aiTextureType_SHININESS,
    Opacity = aiTextureType_OPACITY,
    Displacement = aiTextureType_DISPLACEMENT,
    Lightmap = aiTextureType_LIGHTMAP,
    Reflection = aiTextureType_REFLECTION,
    BaseColor = aiTextureType_BASE_COLOR,
    NormalCamera = aiTextureType_NORMAL_CAMERA,
    EmissionColor = aiTextureType_EMISSION_COLOR,
    Metalness = aiTextureType_METALNESS,
    DiffuseRoughness = aiTextureType_DIFFUSE_ROUGHNESS,
    AmbientOcclusion = aiTextureType_AMBIENT_OCCLUSION,
    Unknown = aiTextureType_UNKNOWN,
    Sheen = aiTextureType_SHEEN,
    Clearcoat = aiTextureType_CLEARCOAT,
    Transmission = aiTextureType_TRANSMISSION,
    MayaBase = aiTextureType_MAYA_BASE,
    MayaSpecular = aiTextureType_MAYA_SPECULAR,
    MayaSpecularColor = aiTextureType_MAYA_SPECULAR_COLOR,
    MayaSpecularRoughness = aiTextureType_MAYA_SPECULAR_ROUGHNESS,
    Max = AI_TEXTURE_TYPE_MAX,
};

class Model {
  public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 texCoord0;

        static VertexDescription Description;
    };

    struct Meta {
        float shininess = 32.0f;
    };

    struct Info {
        std::filesystem::path path = {};
        std::vector<std::tuple<Mesh, Meta>> meshes = {};
        std::vector<Resource<Texture>> textures = {};
    };

    Model(const Info &info);

    bool build(std::shared_ptr<Transfer> transfer);
    bool destroy();

    void draw(const vk::CommandBuffer &commandBuffer);

    const std::vector<std::tuple<Mesh, Meta>> &getMeshs() const;
    const std::vector<Resource<Texture>> &getTextures() const;

    static std::string getTextureTypeName(TextureType texture);

  private:
    bool processNode(const aiNode *ainode, const aiScene *aiscene);
    bool processMesh(const aiMesh *aimesh, const aiScene *scene);
    void loadTextures(const aiMaterial *aimaterial, const aiTextureType type);

    Info m_Info;
};

template <>
struct IsResource<Model> : std::true_type {};

} // namespace Physbuzz
