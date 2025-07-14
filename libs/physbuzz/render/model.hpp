#pragma once

#include "../resources/resources.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include <assimp/scene.h>
#include <filesystem>

namespace Physbuzz {

constexpr std::size_t TextureTypeMax = AI_TEXTURE_TYPE_MAX;
enum class TextureType {
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
};

class Model {
  public:
    struct Meta {
        float shininess = 32.0f;
    };

    struct Info {
        std::filesystem::path path;
        std::vector<std::tuple<Mesh, Meta>> meshes;
        std::vector<Resource<Texture2D>> textures;
    };

    Model(const Info &info);

    bool build();
    bool destroy();

    const std::vector<std::tuple<Mesh, Meta>> &getMeshs() const;
    const std::vector<Resource<Texture2D>> &getTextures() const;

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
