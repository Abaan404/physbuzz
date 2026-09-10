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

    struct MeshResult {
        Mesh::Info info;
        std::vector<std::size_t> submeshMaterialIndices;

        Geometry<Vertex> geometry;
    };

    struct Result {
        MeshResult mesh;
        std::vector<MaterialResult> materials;
    };

    Model(const Info &info);

    std::optional<Result> load(const std::filesystem::path &path, TransferBatch &batch, bool flipUVs = false);

    const Info &getInfo() const;

  private:
    struct SubMeshResult {
        std::uint32_t submeshIdx;
        std::uint32_t materialIdx;

        AABB bounding;
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
    };

    Info m_Info;

    MaterialResult processMaterial(const aiMaterial *aimaterial);
    MeshResult processMesh(const aiNode *ainode, const aiScene *aiscene);
    SubMeshResult processSubMesh(const aiMesh *aimesh, const aiMatrix4x4 &transform);
};

} // namespace Physbuzz
