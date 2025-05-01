#include "model.hpp"

#include "../debug/logging.hpp"
#include "texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Physbuzz {

const glm::vec3 TransformComponent::toWorld(const glm::vec3 &local) const {
    return matrix * glm::vec4(local, 1.0f);
}

const glm::vec3 TransformComponent::toLocal(const glm::vec3 &world) const {
    return glm::inverse(matrix) * glm::vec4(world, 1.0f);
}

void TransformComponent::reset() {
    position = {0.0f, 0.0f, 0.0f};
    scale = {1.0f, 1.0f, 1.0f};
    orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
}

void TransformComponent::update() {
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
    const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::angle(orientation), glm::axis(orientation)); // conjugate?
    const glm::mat4 stretch = glm::scale(glm::mat4(1.0f), scale);

    matrix = translation * rotation * stretch;
}

ModelResource::ModelResource(const std::filesystem::path &path)
    : m_Path(path) {}

ModelResource::ModelResource(const std::vector<Mesh> &meshes, std::unordered_map<TextureType, std::vector<std::string>> &textures)
    : m_Meshes(meshes), m_Textures(textures) {}

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

const std::unordered_map<TextureType, std::vector<std::string>> &ModelResource::getTextures() const {
    return m_Textures;
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

        loadTextures(material, aiTextureType_DIFFUSE);
        loadTextures(material, aiTextureType_SPECULAR);
    }

    m_Meshes.push_back(mesh);

    return true;
}

bool ModelResource::loadTextures(aiMaterial *aimaterial, aiTextureType type) {
    for (unsigned int i = 0; i < aimaterial->GetTextureCount(type); i++) {
        aiString path;
        aimaterial->GetTexture(type, i, &path);

        Texture2DInfo info = {
            .image = {
                .file = {
                    .path = path.C_Str(),
                },
            },
        };

        const std::string &textureName = info.image.file.path.filename().string();

        if (!ResourceRegistry::contains<Texture2DResource>(textureName)) {
            ResourceRegistry::insert(info.image.file.path.filename(), Texture2DResource(info));
        }

        m_Textures[static_cast<TextureType>(type)].push_back(textureName);
    }

    return true;
}

} // namespace Physbuzz
