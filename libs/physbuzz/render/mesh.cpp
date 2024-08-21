#include "mesh.hpp"

namespace Physbuzz {

const glm::vec3 Model::toWorld(const glm::vec3 &local) const {
    return matrix * glm::vec4(local, 1.0f);
}

const glm::vec3 Model::toLocal(const glm::vec3 &world) const {
    return glm::inverse(matrix) * glm::vec4(world, 1.0f);
}

void Model::reset() {
    position = {0.0f, 0.0f, 0.0f};
    scale = {1.0f, 1.0f, 1.0f};
    orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
}

void Model::update() {
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
    const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::angle(orientation), glm::axis(orientation)); // conjugate?
    const glm::mat4 stretch = glm::scale(glm::mat4(1.0f), scale);

    matrix = translation * rotation * stretch;
}

MeshComponent::MeshComponent(const Model &model, const Material &material)
    : model(model), material(material) {}

MeshComponent::~MeshComponent() {}

void MeshComponent::build() {
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, texCoords)));
    glEnableVertexAttribArray(2);
    unbind();

    model.update();
}

void MeshComponent::destroy() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void MeshComponent::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * indices.size(), indices.data(), GL_STREAM_DRAW);
}

void MeshComponent::draw() const {
    glDrawElements(GL_TRIANGLES, sizeof(Index) * indices.size(), GL_UNSIGNED_INT, 0);
}

void MeshComponent::unbind() const {
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace Physbuzz
