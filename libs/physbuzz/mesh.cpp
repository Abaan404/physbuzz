#include "mesh.hpp"

namespace Physbuzz {

RenderComponent::RenderComponent() {}

RenderComponent::RenderComponent(const RenderComponent &other) {
    copy(other);
}

RenderComponent &RenderComponent::operator=(const RenderComponent &other) {
    if (this != &other) {
        copy(other);
    }
    return *this;
}

void RenderComponent::copy(const RenderComponent &other) {
    VBO = other.VBO;
    EBO = other.EBO;
    VAO = other.VAO;

    m_Program = other.m_Program;

    gluTime = other.gluTime;
    gluTimedelta = other.gluTimedelta;
    gluResolution = other.gluResolution;
}

RenderComponent::~RenderComponent() {}

void RenderComponent::build() {
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
}

void RenderComponent::destroy() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void RenderComponent::setProgram(unsigned int program) {
    m_Program = program;

    // common uniforms for every shader
    gluTime = glGetUniformLocation(program, "u_time");
    gluResolution = glGetUniformLocation(program, "u_resolution");
}

std::uint32_t RenderComponent::getProgram() const {
    return m_Program;
}

} // namespace Physbuzz
