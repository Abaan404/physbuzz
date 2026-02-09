#pragma once

#include "../graphics/descriptors/texture.hpp"
#include "../graphics/mesh.hpp"
#include "../resources/resource.hpp"
#include <assimp/scene.h>
#include <filesystem>

namespace Physbuzz {

class Material;

enum class TextureType : std::int32_t {
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

    struct Data {
        Resource<Material> material;
        Resource<Mesh> mesh;
    };

    struct Info {
        std::vector<Data> meshes = {};
    };

    Model(const Info &info);

    bool load(const std::filesystem::path &path, const std::shared_ptr<Transfer> transfer);

    static std::string getTextureTypeName(TextureType texture);
    const Info &getInfo() const;

  private:
    struct MeshResult {
        std::uint32_t meshIdx;
        std::uint32_t materialIdx;

        Mesh::Info<Vertex> info;
    };

    struct TextureResult {
        Texture::Info info;
        std::filesystem::path path;
    };

    struct MaterialResult {
        std::unordered_map<TextureType, TextureResult> textures;
        float shininess;
    };

    Info m_Info;

    MaterialResult processMaterial(const aiMaterial *aimaterial);
    std::vector<MeshResult> processNodes(const aiNode *ainode, const aiScene *aiscene);
    MeshResult processMesh(const aiMesh *aimesh);
};

} // namespace Physbuzz
