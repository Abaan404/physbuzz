#include "model.hpp"

#include "../debug/logging.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

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
    .binding = 0,
}};

Model::Model(const Info &info)
    : m_Info(info) {}

bool Model::load(const std::filesystem::path &path, const std::shared_ptr<Transfer> transfer) {
    // if supplied a path, load the model from the filesystem
    Assimp::Importer importer;
    const aiScene *aiscene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

    if (!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode) {
        Logger::ERROR("[Model] Could not import model at '{}'.\n[Assimp]: {}", path.string(), importer.GetErrorString());
        return false;
    }

    std::vector<MaterialResult> materials;
    for (std::size_t i = 0; i < aiscene->mNumMaterials; i++) {
        aiMaterial *material = aiscene->mMaterials[i];
        materials.emplace_back(processMaterial(material));
    }

    std::vector<MeshResult> meshes = processNodes(aiscene->mRootNode, aiscene);

    for (const auto &mesh : meshes) {
        Data data = {
            .material = std::format("model/{}@{}", mesh.materialIdx, path.string()),
            .mesh = std::format("model/{}@{}", mesh.meshIdx, path.string()),
        };

        if (!ResourceRegistry<Mesh>::insert(data.mesh, mesh.info, transfer)) {
            Logger::ERROR("[Model] Failed to load mesh resource {}.", data.mesh);
            return false;
        }

        Material material = {
            .shininess = materials[mesh.materialIdx].shininess,
            .textures = {},
        };

        for (const auto &[type, textures] : materials[mesh.materialIdx].textures) {
            for (const auto &texture : textures) {

                ImageFile::Info imageFile = {
                    .file = {
                        .path = path / texture.path,
                    },
                };

                ResourceID resourceId = std::format("model@{}", imageFile.file.path.string());

                if (!ResourceRegistry<Texture>::insert(resourceId, texture.info, imageFile, transfer)) {
                    Logger::ERROR("[Model] Failed to load texture resource {}.", resourceId);
                    return false;
                }

                material.textures[type].emplace_back(resourceId);
            }
        }

        if (!ResourceRegistry<Material>::insert(data.material, std::move(material))) {
            Logger::ERROR("[Model] Failed to load material resource {}.", data.material);
            return false;
        }

        m_Info.meshes.emplace_back(data);
    }

    return true;
}

const Model::Info &Model::getInfo() const {
    return m_Info;
};

std::string Model::getTextureTypeName(TextureType texture) {
    return aiTextureTypeToString(static_cast<aiTextureType>(texture));
}

Model::MaterialResult Model::processMaterial(const aiMaterial *aimaterial) {
    MaterialResult result = {
        .textures = {},
        .shininess = 32.0f,
    };

    for (std::size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++) {
        aiTextureType type = static_cast<aiTextureType>(i);

        result.textures[static_cast<TextureType>(type)] = processTextures(aimaterial, type);
    }

    float shininiess;
    PBZ_ASSERT(aimaterial->Get(AI_MATKEY_SHININESS, shininiess) == aiReturn_SUCCESS, "[Model] Failed to material properties.");

    if (shininiess != 0.0f) {
        result.shininess = shininiess;
    }

    return result;
}

std::vector<Model::TextureResult> Model::processTextures(const aiMaterial *aimaterial, aiTextureType type) {
    std::vector<Model::TextureResult> results;

    for (std::uint32_t i = 0; i < aimaterial->GetTextureCount(type); i++) {
        aiString aiPath;

        PBZ_ASSERT(aimaterial->GetTexture(type, i, &aiPath) == aiReturn_SUCCESS, "[Model] Failed to load material texture.");

        results.emplace_back<TextureResult>({
            .info = {},
            .path = aiPath.C_Str(),
        });
    }

    return results;
}

std::vector<Model::MeshResult> Model::processNodes(const aiNode *root, const aiScene *aiscene) {
    std::vector<Model::MeshResult> results;

    std::vector<const aiNode *> stack;
    stack.emplace_back(root);

    while (!stack.empty()) {
        const aiNode *ainode = stack.back();
        stack.pop_back();

        for (std::size_t i = 0; i < ainode->mNumMeshes; ++i) {
            aiMesh *aimesh = aiscene->mMeshes[ainode->mMeshes[i]];
            MeshResult &result = results.emplace_back(processMesh(aimesh));
            result.meshIdx = i;
        }

        for (std::size_t i = ainode->mNumChildren - 1; i < ainode->mNumChildren; i--) {
            stack.emplace_back(ainode->mChildren[i]);
        }
    }

    return results;
}

Model::MeshResult Model::processMesh(const aiMesh *aimesh) {
    MeshResult result = {
        .meshIdx = {},
        .materialIdx = aimesh->mMaterialIndex,
        .info = {},
    };

    result.info.vertices.reserve(aimesh->mNumVertices);
    if (aimesh->HasPositions()) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            result.info.vertices[i].position = {aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z};
        }
    }

    if (aimesh->HasNormals()) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            result.info.vertices[i].normal = {aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z};
        }
    }

    if (aimesh->HasTangentsAndBitangents()) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            result.info.vertices[i].tangent = {aimesh->mTangents[i].x, aimesh->mTangents[i].y, aimesh->mTangents[i].z};
        }
    }

    if (aimesh->HasTextureCoords(0)) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            result.info.vertices[i].texCoord0 = {aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y};
        }
    }

    result.info.indices.reserve(aimesh->mNumFaces * 3);
    if (aimesh->HasFaces()) {
        for (std::size_t i = 0; i < aimesh->mNumFaces; ++i) {
            aiFace &face = aimesh->mFaces[i];

            PBZ_ASSERT(face.mNumIndices % 3 == 0, "[Model] Invalid number of indices for model '{}'");

            for (std::size_t j = 0; j < face.mNumIndices; ++j) {
                result.info.indices.emplace_back(face.mIndices[j]);
            }
        }
    }

    return result;
}

} // namespace Physbuzz
