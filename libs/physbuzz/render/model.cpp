#include "model.hpp"

#include "../debug/logging.hpp"
#include "texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Physbuzz {

ModelResource::ModelResource(const std::filesystem::path &path)
    : m_Path(path) {}

ModelResource::ModelResource(const std::vector<Mesh> &meshes)
    : m_Meshes(meshes) {}

ModelResource::~ModelResource() {}

bool ModelResource::build() {
    // if supplied a path, load the model from the filesystem
    if (!m_Path.empty()) {
        load();
    }

    for (auto &mesh : m_Meshes) {
        mesh.build();
    }

    return true;
}

bool ModelResource::destroy() {
    for (auto &mesh : m_Meshes) {
        mesh.destroy();
    }

    return true;
}

const std::vector<Mesh> &ModelResource::getMeshs() const {
    return m_Meshes;
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

bool ModelResource::processNode(aiNode *ainode, const aiScene *aiscene) {
    for (std::size_t i = 0; i < ainode->mNumMeshes; ++i) {
        aiMesh *mesh = aiscene->mMeshes[ainode->mMeshes[i]];
        processMesh(mesh, aiscene);
    }

    for (std::size_t i = 0; i < ainode->mNumChildren; ++i) {
        processNode(ainode->mChildren[i], aiscene);
    }

    return true;
}

bool ModelResource::processMesh(aiMesh *aimesh, const aiScene *scene) {
    Mesh mesh;
    mesh.vertices.resize(aimesh->mNumVertices);

    // pos and norm
    for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
        mesh.vertices[i].position = {aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z};
        mesh.vertices[i].normal = {aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z};
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

    // texcoords
    if (aimesh->mTextureCoords[0]) {
        for (std::size_t i = 0; i < aimesh->mNumVertices; ++i) {
            mesh.vertices[i].texCoords = {aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y};
        }
    }

    // material
    if (aimesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];

        mesh.textures[TextureType::Diffuse] = loadTextures(material, aiTextureType_DIFFUSE);
        mesh.textures[TextureType::Specular] = loadTextures(material, aiTextureType_SPECULAR);

        material->Get(AI_MATKEY_SHININESS, mesh.shininess);
    }

    m_Meshes.push_back(mesh);

    return true;
}

std::vector<ResourceHandle<Texture2DResource>> ModelResource::loadTextures(aiMaterial *aimaterial, aiTextureType type) {
    std::uint32_t size = aimaterial->GetTextureCount(type);

    std::vector<ResourceHandle<Texture2DResource>> textures;
    textures.reserve(size);

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
        };

        if (!ResourceRegistry<Texture2DResource>::contains(path)) {
            ResourceRegistry<Texture2DResource>::insert(path, info);
        }

        textures.emplace_back(path);
    }

    return textures;
}

} // namespace Physbuzz
