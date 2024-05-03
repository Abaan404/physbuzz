#include "framebuffer.hpp"

#include <glad/gl.h>

namespace Physbuzz {

Framebuffer::Framebuffer(glm::ivec2 &resolution) : m_Resolution(resolution) {}

Framebuffer::Framebuffer(const Framebuffer &other) : m_Resolution(other.m_Resolution) {
    if (this != &other) {
        m_Framebuffer = other.m_Framebuffer;
        m_Depth = other.m_Depth;
        m_Color = other.m_Color;
    }
}

Framebuffer Framebuffer::operator=(const Framebuffer &other) {
    if (this != &other) {
        m_Resolution = other.m_Resolution;

        m_Framebuffer = other.m_Framebuffer;
        m_Depth = other.m_Depth;
        m_Color = other.m_Color;
    }

    return *this;
}

Framebuffer::~Framebuffer() {}

void Framebuffer::build() {
    glGenFramebuffers(1, &m_Framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

    glGenTextures(1, &m_Color);
    glBindTexture(GL_TEXTURE_2D, m_Color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Resolution.x, m_Resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Color, 0);

    glGenRenderbuffers(1, &m_Depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_Depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Resolution.x, m_Resolution.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Depth);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy() {
    glDeleteFramebuffers(1, &m_Framebuffer);
    glDeleteTextures(1, &m_Depth);
    glDeleteRenderbuffers(1, &m_Color);
}

void Framebuffer::resize(glm::ivec2 &resolution) {
    m_Resolution = resolution;

    glBindTexture(GL_TEXTURE_2D, m_Color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_Depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Framebuffer::clear(glm::vec4 &color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
    glViewport(0, 0, m_Resolution.x, m_Resolution.y);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Framebuffer::getColor() const {
    return m_Color;
};

unsigned int Framebuffer::getDepth() const {
    return m_Depth;
};

unsigned int Framebuffer::getFramebuffer() const {
    return m_Framebuffer;
}

glm::ivec2 Framebuffer::getResolution() const {
    return m_Resolution;
};

} // namespace Physbuzz
