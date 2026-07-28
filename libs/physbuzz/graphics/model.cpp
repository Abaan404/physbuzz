#include "model.hpp"

#include "../io/image.hpp"
#include "../io/logging.hpp"
#include "material.hpp"
#include "transfer.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <tracy/Tracy.hpp>

namespace Physbuzz {

VertexDescription Model::Vertex::Description = {{
    .attributes = {
        {
            .format = VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(Vertex::position) / sizeof(decltype(Vertex::position)::value_type),
            .offset = offsetof(Vertex, position),
        },
        {
            .format = VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(Vertex::normal) / sizeof(decltype(Vertex::normal)::value_type),
            .offset = offsetof(Vertex, normal),
        },
        {
            .format = VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(Vertex::tangent) / sizeof(decltype(Vertex::tangent)::value_type),
            .offset = offsetof(Vertex, tangent),
        },
        {
            .format = VertexDescription::Format::eR32G32Sfloat,
            .size = sizeof(Vertex::texCoord0) / sizeof(decltype(Vertex::texCoord0)::value_type),
            .offset = offsetof(Vertex, texCoord0),
        },
    },
    .size = sizeof(Vertex),
}};

Model::Model(const Info &info)
    : m_Info(info) {}

bool Model::load(const std::filesystem::path &path, const std::shared_ptr<Transfer> transfer, bool flipTexturesVertically) {
    ZoneScopedN("Model/Load");
    ZoneText(path.string().c_str(), path.string().size());

    m_Info = {
        .mesh = {""},
        .materials = {},
        .submeshMaterialIndices = {},
    };

    // if supplied a path, load the model from the filesystem
    Assimp::Importer importer;
    const aiScene *aiscene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if (!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode) {
        Logger::ERROR("[Model] Could not import model at '{}'.\n[Assimp]: {}", path.string(), importer.GetErrorString());
        return false;
    }

    TransferBatch batch = {{}};

    for (std::size_t i = 0; i < aiscene->mNumMaterials; i++) {
        ResourceID resourceId = std::format("model/{}@{}", i, path.string());

        MaterialResult materialResult = processMaterial(aiscene->mMaterials[i]);

        Material material;

        for (const auto &[type, texture] : materialResult.textures) {
            std::filesystem::path texturePath = path.parent_path() / texture.path;

            ResourceID resourceId = std::format("model@{}", texturePath.string());

            // a material can reference the same texture again
            if (!ResourceRegistry<Texture>::contains(resourceId)) {
                ImageFile imageFile = {{
                    .files = {{
                        .path = texturePath,
                    }},
                    .flipVertically = flipTexturesVertically,
                }};

                imageFile.readMeta();

                if (!ResourceRegistry<Texture>::insert(resourceId, texture.info, glm::uvec3{imageFile.getData().meta.resolution, 1})) {
                    Logger::ERROR("[Model] Failed to build texture resource {}.", resourceId);
                    return false;
                }

                if (!Resource<Texture>{resourceId}->write(imageFile.getInfo(), batch)) {
                    Logger::ERROR("[Model] Failed to write texture resource {}.", resourceId);
                    return false;
                }
            }

            material.textures.insert({type, resourceId});
        }

        if (!ResourceRegistry<Material>::contains(resourceId)) {
            if (!ResourceRegistry<Material>::insert(resourceId, std::move(material))) {
                Logger::ERROR("[Model] Failed to load material resource {}.", resourceId);
                return false;
            }
        }

        m_Info.materials.emplace_back(resourceId);
    }

    MeshResult meshResult = processMesh(aiscene->mRootNode, aiscene);

    m_Info.mesh = std::format("model@{}", path.string());

    if (!ResourceRegistry<Mesh>::contains(m_Info.mesh)) {
        if (!ResourceRegistry<Mesh>::insert(m_Info.mesh, meshResult.info)) {
            Logger::ERROR("[Model] Failed to build mesh resource {}.", m_Info.mesh);
            return false;
        }

        if (!Resource<Mesh>{m_Info.mesh}->write(
                std::move(meshResult.vertices),
                std::move(meshResult.indices),
                batch)) {
            Logger::ERROR("[Model] Failed to write mesh resource {}.", m_Info.mesh);
            return false;
        }
    }

    m_Info.submeshMaterialIndices = meshResult.submeshMaterialIndices;

    transfer->submit(batch);

    return true;
}

const Model::Info &Model::getInfo() const {
    return m_Info;
}

Model::MaterialResult Model::processMaterial(const aiMaterial *aimaterial) {
    ZoneScopedN("Model/ProcessMaterial");

    MaterialResult result;

    for (std::size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++) {
        aiTextureType type = static_cast<aiTextureType>(i);

        if (aimaterial->GetTextureCount(type) >= 1) {
            // only fetch the first texture found
            aiString aiPath;
            aiReturn successPath = aimaterial->GetTexture(type, 0, &aiPath);
            PBZ_ASSERT(successPath == aiReturn_SUCCESS, "[Model] Failed to load material texture.");

            TextureType typePbz = TextureType::Unknown;

            switch (type) {
            case aiTextureType_BASE_COLOR:
            case aiTextureType_DIFFUSE:
                typePbz = TextureType::Albedo;
                break;

            case aiTextureType_HEIGHT:
            case aiTextureType_NORMALS:
                typePbz = TextureType::Normal;
                break;

            case aiTextureType_METALNESS:
            case aiTextureType_SPECULAR:
            case aiTextureType_SHININESS:
                typePbz = TextureType::Metallic;
                break;

            case aiTextureType_DIFFUSE_ROUGHNESS:
                typePbz = TextureType::Roughness;
                break;

            case aiTextureType_EMISSIVE:
            case aiTextureType_EMISSION_COLOR:
                typePbz = TextureType::Emission;
                break;

            case aiTextureType_AMBIENT_OCCLUSION:
                typePbz = TextureType::AmbientOcclusion;
                break;

            default:
                Logger::WARNING("[MaterialAllocator] Unhandled texture type {}", aiTextureTypeToString(type));
                continue;
            }

            result.textures[typePbz] = {
                .info = {
                    .type = Texture::Type::Dim2D,
                    .sampler = {{Physbuzz::Sampler::Type::Linear}},
                },
                .path = aiPath.C_Str(),
            };
        }
    }

    aimaterial->Get(AI_MATKEY_BASE_COLOR, result.albedoFactor);
    aimaterial->Get(AI_MATKEY_METALLIC_FACTOR, result.metallicFactor);
    aimaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, result.roughnessFactor);

    return result;
}

Model::MeshResult Model::processMesh(const aiNode *root, const aiScene *aiscene) {
    ZoneScopedN("Model/ProcessNodes");

    std::vector<SubMeshResult> submeshes;

    MeshResult result = {
        .info = {
            .description = &Vertex::Description,
            .vertexCount = 0,
            .indexCount = 0,
            .submeshes = {},
        },
        .submeshMaterialIndices = {},
        .vertices = {},
        .indices = {},
    };

    std::vector<std::pair<const aiNode *, aiMatrix4x4>> stack;
    stack.emplace_back(root, root->mTransformation);

    while (!stack.empty()) {
        auto [ainode, transform] = stack.back();
        stack.pop_back();

        result.info.submeshes.reserve(result.info.submeshes.size() + ainode->mNumMeshes);

        for (std::size_t i = 0; i < ainode->mNumMeshes; ++i) {
            aiMesh *aimesh = aiscene->mMeshes[ainode->mMeshes[i]];
            SubMeshResult &submesh = submeshes.emplace_back(processSubMesh(aimesh));

            AABB::Info aabb = {};
            for (const auto vertex : submesh.vertices) {
                aabb.min = glm::min(vertex.position, aabb.min);
                aabb.max = glm::max(vertex.position, aabb.max);
            }

            submesh.bounding = {aabb};
            submesh.submeshIdx = submeshes.size() - 1;

            result.info.submeshes.emplace_back<Mesh::SubMesh>({
                .bounding = submesh.bounding,
                .indexCount = static_cast<std::uint32_t>(submesh.indices.size()),
                .firstIndex = result.info.indexCount,
                .vertexOffset = static_cast<std::int32_t>(result.info.vertexCount),
            });

            result.info.indexCount += submesh.indices.size();
            result.info.vertexCount += submesh.vertices.size();
        }

        for (std::size_t i = ainode->mNumChildren - 1; i < ainode->mNumChildren; i--) {
            const aiNode *child = ainode->mChildren[i];
            stack.emplace_back(child, transform * child->mTransformation);
        }
    }

    result.vertices.reserve(result.info.vertexCount);
    result.indices.reserve(result.info.indexCount);

    for (const auto &submesh : submeshes) {
        result.submeshMaterialIndices.emplace_back(submesh.materialIdx);

        result.vertices.insert(result.vertices.end(), submesh.vertices.begin(), submesh.vertices.end());
        result.indices.insert(result.indices.end(), submesh.indices.begin(), submesh.indices.end());
    }

    return result;
}

Model::SubMeshResult Model::processSubMesh(const aiMesh *aimesh) {
    ZoneScopedN("Model/ProcessMesh");

    SubMeshResult result = {
        .submeshIdx = {},
        .materialIdx = aimesh->mMaterialIndex,
        .bounding = {},
        .vertices = {},
        .indices = {},
    };

    result.vertices.resize(aimesh->mNumVertices);
    if (aimesh->HasPositions()) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            result.vertices[i].position = {aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z};
        }
    }

    if (aimesh->HasNormals()) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            result.vertices[i].normal = {aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z};
        }
    }

    if (aimesh->HasTangentsAndBitangents()) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            result.vertices[i].tangent = {aimesh->mTangents[i].x, aimesh->mTangents[i].y, aimesh->mTangents[i].z};
        }
    }

    if (aimesh->HasTextureCoords(0)) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            result.vertices[i].texCoord0 = {aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y};
        }
    }

    result.indices.resize(aimesh->mNumFaces * 3);
    if (aimesh->HasFaces()) {
        for (std::size_t i = 0; i < aimesh->mNumFaces; ++i) {
            aiFace &face = aimesh->mFaces[i];

            PBZ_ASSERT(face.mNumIndices == 3, "[Model] Invalid number of indices in face for model '{}'");

            result.indices[i * 3] = face.mIndices[0];
            result.indices[i * 3 + 1] = face.mIndices[1];
            result.indices[i * 3 + 2] = face.mIndices[2];
        }
    }

    return result;
}

} // namespace Physbuzz
