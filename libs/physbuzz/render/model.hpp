#pragma once

#include "../resources/handle.hpp"
#include "mesh.hpp"
#include <assimp/scene.h>
#include <filesystem>

namespace Physbuzz {

class ModelResource {
  public:
    ModelResource(const std::filesystem::path &path);
    ModelResource(const std::vector<Mesh> &meshes);
    ~ModelResource();

    bool build();
    bool destroy();

    const std::vector<Mesh> &getMeshs() const;

  private:
    bool load();

    bool processNode(aiNode *ainode, const aiScene *aiscene);
    bool processMesh(aiMesh *aimesh, const aiScene *scene);
    std::vector<ResourceHandle<Texture2DResource>> loadTextures(aiMaterial *aimaterial, aiTextureType type);

    std::filesystem::path m_Path;
    std::vector<Mesh> m_Meshes;
};

struct ModelComponent {
    ResourceHandle<ModelResource> model;
};

} // namespace Physbuzz
