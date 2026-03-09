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

bool Model::load(const std::filesystem::path &path, const std::shared_ptr<Transfer> transfer) {
    ZoneScopedN("Model/Load");
    ZoneText(path.string().c_str(), path.string().size());

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

        Material material = {
            .shininess = materialResult.shininess,
            .textures = {},
        };

        for (const auto &[type, texture] : materialResult.textures) {
            std::filesystem::path texturePath = path.parent_path() / texture.path;

            ResourceID resourceId = std::format("model@{}", texturePath.string());

            // a material can reference the same texture again
            if (ResourceRegistry<Texture>::contains(resourceId)) {
                continue;
            }

            ImageFile imageFile = {{.files = {{.path = texturePath}}}};
            imageFile.readMeta();

            if (!ResourceRegistry<Texture>::insert(resourceId, texture.info, glm::uvec3{imageFile.getData().meta.resolution, 1})) {
                Logger::ERROR("[Model] Failed to build texture resource {}.", resourceId);
                return false;
            }

            material.textures.insert({type, resourceId});
            if (!Resource<Texture>{resourceId}->write({.files = {{.path = texturePath}}}, batch)) {
                Logger::ERROR("[Model] Failed to write texture resource {}.", resourceId);
                return false;
            }
        }

        if (!ResourceRegistry<Material>::insert(resourceId, std::move(material))) {
            Logger::ERROR("[Model] Failed to load material resource {}.", resourceId);
            return false;
        }
    }

    std::vector<MeshResult> meshResults = processNodes(aiscene->mRootNode, aiscene);

    for (std::size_t i = 0; i < meshResults.size(); i++) {
        Data data = {
            .material = std::format("model/{}@{}", meshResults[i].materialIdx, path.string()),
            .mesh = std::format("model/{}@{}", meshResults[i].meshIdx, path.string()),
        };

        if (!ResourceRegistry<Mesh>::insert(data.mesh, meshResults[i].info)) {
            Logger::ERROR("[Model] Failed to build mesh resource {}.", data.mesh);
            return false;
        }

        if (!Resource<Mesh>{data.mesh}->write(std::move(meshResults[i].vertices), std::move(meshResults[i].indices), batch)) {
            Logger::ERROR("[Model] Failed to write mesh resource {}.", data.mesh);
            return false;
        }

        m_Info.meshes.emplace_back(data);
    }

    transfer->submit(batch);

    return true;
}

const Model::Info &Model::getInfo() const {
    return m_Info;
}

std::string Model::getTextureTypeName(TextureType texture) {
    return aiTextureTypeToString(static_cast<aiTextureType>(texture));
}

Model::MaterialResult Model::processMaterial(const aiMaterial *aimaterial) {
    ZoneScopedN("Model/ProcessMaterial");

    MaterialResult result = {
        .textures = {},
        .shininess = 32.0f,
    };

    for (std::size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++) {
        aiTextureType type = static_cast<aiTextureType>(i);

        if (aimaterial->GetTextureCount(type) >= 1) {
            // only fetch the first texture found
            aiString aiPath;
            aiReturn successPath = aimaterial->GetTexture(type, 0, &aiPath);
            PBZ_ASSERT(successPath == aiReturn_SUCCESS, "[Model] Failed to load material texture.");

            result.textures[static_cast<TextureType>(type)] = {
                .info = {
                    .type = Texture::Type::Dim2D,
                    .sampler = {{Physbuzz::Sampler::Type::Linear}},
                },
                .path = aiPath.C_Str(),
            };
        }
    }

    float shininiess;
    aiReturn successShininess = aimaterial->Get(AI_MATKEY_SHININESS, shininiess);
    PBZ_ASSERT(successShininess == aiReturn_SUCCESS, "[Model] Failed to material properties.");

    if (shininiess != 0.0f) {
        result.shininess = shininiess;
    }

    return result;
}

std::vector<Model::MeshResult> Model::processNodes(const aiNode *root, const aiScene *aiscene) {
    ZoneScopedN("Model/ProcessNodes");

    std::vector<MeshResult> results;

    std::vector<std::pair<const aiNode *, aiMatrix4x4>> stack;
    stack.emplace_back(root, root->mTransformation);

    while (!stack.empty()) {
        auto [ainode, transform] = stack.back();
        stack.pop_back();

        for (std::size_t i = 0; i < ainode->mNumMeshes; ++i) {
            aiMesh *aimesh = aiscene->mMeshes[ainode->mMeshes[i]];
            MeshResult &result = results.emplace_back(processMesh(aimesh));
            result.meshIdx = results.size() - 1;
        }

        for (std::size_t i = ainode->mNumChildren - 1; i < ainode->mNumChildren; i--) {
            const aiNode *child = ainode->mChildren[i];
            stack.emplace_back(child, transform * child->mTransformation);
        }
    }

    return results;
}

Model::MeshResult Model::processMesh(const aiMesh *aimesh) {
    ZoneScopedN("Model/ProcessMesh");

    MeshResult result = {
        .meshIdx = {},
        .materialIdx = aimesh->mMaterialIndex,
        .info = {
            .description = &Vertex::Description,
            .vertexCount = aimesh->mNumVertices,
            .indexCount = aimesh->mNumFaces * 3,
        },
        .vertices = {},
        .indices = {},
    };

    result.vertices.resize(result.info.vertexCount);
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

    result.indices.resize(result.info.indexCount);
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
