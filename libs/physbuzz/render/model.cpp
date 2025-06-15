#include "model.hpp"

#include "../debug/logging.hpp"
#include "texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Physbuzz {

ModelResource::ModelResource(const std::filesystem::path &path)
    : m_Path(path) {}

ModelResource::ModelResource(const ModelInfo &info)
    : m_Info(info) {}

ModelResource::~ModelResource() {}

bool ModelResource::build() {
    // if supplied a path, load the model from the filesystem
    if (!m_Path.empty()) {
        load();
    }

    for (auto &[mesh, _] : m_Info.meshes) {
        mesh.build();
    }

    return true;
}

bool ModelResource::destroy() {
    for (auto &[mesh, _] : m_Info.meshes) {
        mesh.destroy();
    }

    return true;
}

bool ModelResource::load() {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(m_Path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Logger::ERROR("[Model] Could not import model \"{}\". Assimp Error: \n{}", m_Path.string(), importer.GetErrorString());
        return false;
    }

    return processNode(scene->mRootNode, scene);
}

bool ModelResource::processNode(const aiNode *ainode, const aiScene *aiscene) {
    for (std::size_t i = 0; i < ainode->mNumMeshes; ++i) {
        aiMesh *mesh = aiscene->mMeshes[ainode->mMeshes[i]];
        processMesh(mesh, aiscene);
    }

    for (std::size_t i = 0; i < ainode->mNumChildren; ++i) {
        processNode(ainode->mChildren[i], aiscene);
    }

    return true;
}

bool ModelResource::processMesh(const aiMesh *aimesh, const aiScene *scene) {
    MeshInfo mesh;
    MeshMeta meta;

    // pos and norm
    mesh.vertices.resize(aimesh->mNumVertices);
    for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
        mesh.vertices[i].position = {aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z};
        mesh.vertices[i].normal = {aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z};
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
            Logger::ERROR("[ModelResource] Invalid number of indices for model \"{}\"");
            return false;
        }

        for (std::size_t j = 0; j < aimesh->mFaces[i].mNumIndices; ++j) {
            mesh.indices.push_back(face.mIndices[j]);
        }
    }

    // material
    if (aimesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];

        loadTextures(material, aiTextureType_DIFFUSE);
        loadTextures(material, aiTextureType_SPECULAR);

        float shininiess;
        material->Get(AI_MATKEY_SHININESS, shininiess);
        if (shininiess != 0.0f) {
            meta.shininess = shininiess;
        }
    }

    m_Info.meshes.emplace_back(mesh, meta);

    return true;
}

void ModelResource::loadTextures(const aiMaterial *aimaterial, aiTextureType type) {
    std::uint32_t size = aimaterial->GetTextureCount(type);

    for (std::uint32_t i = 0; i < size; i++) {
        aiString aiPath;
        aimaterial->GetTexture(type, i, &aiPath);

        std::string path = m_Path.parent_path() / aiPath.C_Str();

        Texture2DInfo info = {
            .image = {
                .file = {
                    .path = path,
                },
            },
            .type = static_cast<TextureType>(type),
        };

        if (!ResourceRegistry<Texture2DResource>::contains(path)) {
            ResourceRegistry<Texture2DResource>::insert(path, info);
            m_Info.textures.emplace_back(path);
        }
    }
}

const std::vector<std::tuple<Mesh, MeshMeta>> &ModelResource::getMeshs() const {
    return m_Info.meshes;
}

const std::vector<ResourceHandle<Texture2DResource>> &ModelResource::getTextures() const {
    return m_Info.textures;
}

} // namespace Physbuzz
