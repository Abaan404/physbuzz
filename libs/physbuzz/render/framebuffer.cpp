#include "framebuffer.hpp"

#include "../debug/logging.hpp"

namespace Physbuzz {

Framebuffer::Framebuffer(const FramebufferInfo &info)
    : m_Info(info) {}

void Framebuffer::build() {
    glGenFramebuffers(1, &m_Framebuffer);

    bind();

    glGenTextures(1, &m_Color);
    glBindTexture(GL_TEXTURE_2D, m_Color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Info.resolution.x, m_Info.resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Color, 0);

    glGenRenderbuffers(1, &m_Depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_Depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Info.resolution.x, m_Info.resolution.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Depth);

    PBZ_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "[Framebuffer] Incomplete Framebuffer.");

    unbind();
}

void Framebuffer::destroy() {
    glDeleteFramebuffers(1, &m_Framebuffer);
    glDeleteTextures(1, &m_Color);
    glDeleteRenderbuffers(1, &m_Depth);
}

void Framebuffer::resize(const glm::ivec2 &resolution) {
    bind();

    glBindTexture(GL_TEXTURE_2D, m_Color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_Depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    PBZ_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "[Framebuffer] Incomplete Framebuffer after resize.");

    glViewport(0, 0, resolution.x, resolution.y);

    unbind();

    m_Info.resolution = resolution;
}

void Framebuffer::clear() {
    glClearColor(m_Info.colorClear.r, m_Info.colorClear.g, m_Info.colorClear.b, m_Info.colorClear.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Framebuffer::getColor() const {
    return m_Color;
}

GLuint Framebuffer::getDepth() const {
    return m_Depth;
}

GLuint Framebuffer::getFramebuffer() const {
    return m_Framebuffer;
}

const glm::ivec2 &Framebuffer::getResolution() const {
    return m_Info.resolution;
}

} // namespace Physbuzz
