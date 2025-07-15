#include "model.hpp"

#include "../debug/logging.hpp"
#include "../resources/builtins/vertices.hpp"
#include "texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Physbuzz {

Model::Model(const Info &info)
    : m_Info(info) {}

bool Model::build() {
    bool success = true;

    // if supplied a path, load the model from the filesystem
    if (!m_Info.path.empty()) {
        success &= Builtin::VertexDefault::build();

        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(m_Info.path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            Logger::ERROR("[Model] Could not import model at \"{}\". Assimp Error: \n{}", m_Info.path.string(), importer.GetErrorString());
            return false;
        }

        success &= processNode(scene->mRootNode, scene);
    }

    for (auto &[mesh, _] : m_Info.meshes) {
        success &= mesh.build();
    }

    return true;
}

bool Model::destroy() {
    for (auto &[mesh, _] : m_Info.meshes) {
        mesh.destroy();
    }

    return true;
}

bool Model::processNode(const aiNode *ainode, const aiScene *aiscene) {
    for (std::size_t i = 0; i < ainode->mNumMeshes; ++i) {
        aiMesh *mesh = aiscene->mMeshes[ainode->mMeshes[i]];
        processMesh(mesh, aiscene);
    }

    for (std::size_t i = 0; i < ainode->mNumChildren; ++i) {
        processNode(ainode->mChildren[i], aiscene);
    }

    return true;
}

bool Model::processMesh(const aiMesh *aimesh, const aiScene *scene) {
    Mesh::Info<Builtin::VertexDefault::Format> mesh = {
        .attribute = Builtin::VertexDefault::Resource.getIdentifier(),
        .vertices = {},
        .indices = {},
    };
    Meta meta;

    // pos and norm
    mesh.vertices.resize(aimesh->mNumVertices);
    for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
        mesh.vertices[i].position = {aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z};
        mesh.vertices[i].normal = {aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z};
        mesh.vertices[i].tangent = {aimesh->mTangents[i].x, aimesh->mTangents[i].y, aimesh->mTangents[i].z};
    }

    // texcoords
    if (aimesh->mTextureCoords[0]) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            mesh.vertices[i].texCoords = {aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y};
        }
    }

    // indices
    for (std::size_t i = 0; i < aimesh->mNumFaces; ++i) {
        aiFace face = aimesh->mFaces[i];

        if (face.mNumIndices % 3 != 0) {
            Logger::ERROR("[Model] Invalid number of indices for model \"{}\"");
            return false;
        }

        for (std::size_t j = 0; j < aimesh->mFaces[i].mNumIndices; ++j) {
            mesh.indices.push_back(face.mIndices[j]);
        }
    }

    // material
    if (aimesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];

        for (std::size_t i = 0; i < TextureTypeMax; i++) {
            loadTextures(material, static_cast<aiTextureType>(i));
        }

        float shininiess;
        material->Get(AI_MATKEY_SHININESS, shininiess);
        if (shininiess != 0.0f) {
            meta.shininess = shininiess;
        }
    }

    m_Info.meshes.emplace_back(mesh, meta);

    return true;
}

void Model::loadTextures(const aiMaterial *aimaterial, aiTextureType type) {
    std::uint32_t size = aimaterial->GetTextureCount(type);

    std::string name = aiTextureTypeToString(type);

    for (std::uint32_t i = 0; i < size; i++) {
        aiString aiPath;
        aimaterial->GetTexture(type, i, &aiPath);

        std::string path = m_Info.path.parent_path() / aiPath.C_Str();

        Texture2D::Info info = {
            .image = {
                .file = {
                    .path = path,
                },
            },
            .type = static_cast<TextureType>(type),
        };

        ResourceID name = std::format("model@{}", path);

        if (!ResourceRegistry<Texture2D>::contains(name)) {
            ResourceRegistry<Texture2D>::insert(name, info);
            m_Info.textures.emplace_back(name);
        }
    }
}

const std::vector<std::tuple<Mesh, Model::Meta>> &Model::getMeshs() const {
    return m_Info.meshes;
}

const std::vector<Resource<Texture2D>> &Model::getTextures() const {
    return m_Info.textures;
}

std::string Model::getTextureTypeName(TextureType texture) {
    return aiTextureTypeToString(static_cast<aiTextureType>(texture));
}

} // namespace Physbuzz
