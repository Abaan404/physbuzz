#pragma once

#include "../resources/resource.hpp"
#include "descriptors/texture.hpp"
#include "mesh.hpp"
#include <assimp/scene.h>
#include <filesystem>

namespace Physbuzz {

class Material;

enum class TextureType {
    Albedo,
    Normal,
    Metallic,
    Roughness,
    Emission,
    AmbientOcclusion,
    Unknown,
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

    struct Info {
        Resource<Mesh> mesh;
        std::vector<Resource<Material>> materials;
        std::vector<std::size_t> submeshMaterialIndices;
    };

    Model(const Info &info);

    bool load(const std::filesystem::path &path, const std::shared_ptr<Transfer> transfer, bool flipUVs = false);

    const Info &getInfo() const;

  private:
    struct SubMeshResult {
        std::uint32_t submeshIdx;
        std::uint32_t materialIdx;

        AABB bounding;
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
    };

    struct MeshResult {
        Mesh::Info info;
        std::vector<std::size_t> submeshMaterialIndices;

        std::vector<Vertex> vertices;
        std::vector<Index> indices;
    };

    struct TextureResult {
        Texture::Info info;
        std::filesystem::path path;
    };

    struct MaterialResult {
        std::unordered_map<TextureType, TextureResult> textures;

        float albedoFactor;
        float metallicFactor;
        float roughnessFactor;
    };

    Info m_Info;

    MaterialResult processMaterial(const aiMaterial *aimaterial);
    MeshResult processMesh(const aiNode *ainode, const aiScene *aiscene);
    SubMeshResult processSubMesh(const aiMesh *aimesh, const aiMatrix4x4 &transform);
};

} // namespace Physbuzz
