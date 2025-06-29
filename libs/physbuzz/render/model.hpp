#pragma once

#include "../resources/resources.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include <assimp/scene.h>
#include <filesystem>

namespace Physbuzz {

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

struct MeshMeta {
    float shininess = 32.0f;
};

struct ModelInfo {
    std::vector<std::tuple<Mesh, MeshMeta>> meshes;
    std::vector<Resource<Texture2D>> textures;
};

class Model : public ResourceTag {
  public:
    Model(const std::filesystem::path &path);
    Model(const ModelInfo &info);
    ~Model();

    bool build();
    bool destroy();

    const std::vector<std::tuple<Mesh, MeshMeta>> &getMeshs() const;
    const std::vector<Resource<Texture2D>> &getTextures() const;

  private:
    bool load();

    bool processNode(const aiNode *ainode, const aiScene *aiscene);
    bool processMesh(const aiMesh *aimesh, const aiScene *scene);
    void loadTextures(const aiMaterial *aimaterial, const aiTextureType type);

    std::filesystem::path m_Path;
    ModelInfo m_Info;
};

} // namespace Physbuzz
