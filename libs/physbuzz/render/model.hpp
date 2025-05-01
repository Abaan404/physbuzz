#pragma once

#include "mesh.hpp"
#include <assimp/scene.h>
#include <filesystem>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

namespace Physbuzz {

enum class TextureType {
    Raw,
    Diffuse,
    Specular,
    Ambient,
    Emissive,
    Height,
    Normals,
    Shininess,
    Opacity,
    Displacement,
    Lightmap,
    Reflection,
    BaseColor,
    NormalCamera,
    EmissionColor,
    Metalness,
    DiffuseRoughness,
    AmbientOcclusion,
    Sheen,
    Clearcoat,
    Transmission,
    Unknown,
};

struct TransformComponent {
    void update();
    void reset();

    const glm::vec3 toWorld(const glm::vec3 &local) const;
    const glm::vec3 toLocal(const glm::vec3 &world) const;

    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 matrix = glm::mat4(1.0f);
};

struct ModelComponent {
    std::string model;
    std::string pipeline;
};

class ModelResource {
  public:
    ModelResource(const std::filesystem::path &path);
    ModelResource(const std::vector<Mesh> &meshes, std::unordered_map<TextureType, std::vector<std::string>> &textures);
    ~ModelResource();

    bool build();
    bool destroy();

    const std::vector<Mesh> &getMeshs() const;
    const std::unordered_map<TextureType, std::vector<std::string>> &getTextures() const;

  private:
    bool load();

    bool processNode(aiNode *ainode, const aiScene *aiscene);
    bool processMesh(aiMesh *aimesh, const aiScene *scene);
    bool loadTextures(aiMaterial *aimaterial, aiTextureType type);

    std::filesystem::path m_Path;
    std::vector<Mesh> m_Meshes;

    std::unordered_map<TextureType, std::vector<std::string>> m_Textures;
};

} // namespace Physbuzz
