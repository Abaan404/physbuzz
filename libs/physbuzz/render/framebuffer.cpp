#include "framebuffer.hpp"

#include "../debug/logging.hpp"
#include "gl/units.hpp"

namespace Physbuzz {

Framebuffer::Framebuffer(const FramebufferInfo &info)
    : m_Info(info) {}

bool Framebuffer::build() {
    if (m_Framebuffer != 0) {
        Logger::WARNING("[Framebuffer] Trying to build a built framebuffer");
        return false;
    }

    GLint maxColors = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColors);

    if (m_Info.colors.size() > maxColors) {
        Logger::ERROR(std::format("[Framebuffer] Too many color attachments, max supported is {}", maxColors));
        return false;
    }

    switch (m_Info.output.type) {
    case OutputAttachmentInfo::Type::Color:
        if (m_Info.output.colorIndex > m_Info.colors.size() - 1) {
            Logger::ERROR(std::format("[Framebuffer] Invalid output color index {}", m_Info.output.colorIndex));
            return false;
        }

        if (!m_Info.colors[m_Info.output.colorIndex].isDrawn || m_Info.colors[m_Info.output.colorIndex].storage != ColorAttachmentInfo::Storage::Texture) {
            Logger::ERROR(std::format("[Framebuffer] output color index cannot be drawn {}", m_Info.output.colorIndex));
            return false;
        }
        break;

    case OutputAttachmentInfo::Type::Depth:
        if (m_Info.depth.storage != DepthAttachmentInfo::Storage::Texture) {
            Logger::ERROR("[Framebuffer] output depth cannot be drawn");
            return false;
        }
        break;
    }

    glGenFramebuffers(1, &m_Framebuffer);
    bind();

    GLuint attachmentIndex = 0;

    std::vector<GLenum> drawBuffers;
    for (const auto &attachment : m_Info.colors) {
        GLuint color = 0;

        switch (attachment.storage) {
        case ColorAttachmentInfo::Storage::Texture:
            glGenTextures(1, &color);
            glBindTexture(GL_TEXTURE_2D, color);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Info.resolution.x, m_Info.resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_TEXTURE_2D, color, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            break;

        case ColorAttachmentInfo::Storage::Renderbuffer:
            glGenRenderbuffers(1, &color);
            glBindRenderbuffer(GL_RENDERBUFFER, color);

            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_Info.resolution.x, m_Info.resolution.y);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_RENDERBUFFER, color);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            break;

        case ColorAttachmentInfo::Storage::None:
            Logger::ERROR("[Framebuffer] No storage declared for the current color attachment");
            destroy();
            return false;
        }

        if (attachment.isDrawn) {
            drawBuffers.emplace_back(GL_COLOR_ATTACHMENT0 + attachmentIndex);
        }

        attachmentIndex++;
        m_Colors.emplace_back(color);
    }

    if (m_Colors.size() > 0) {
        glDrawBuffers(drawBuffers.size(), drawBuffers.data());
    } else {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    switch (m_Info.depth.storage) {
    case DepthAttachmentInfo::Storage::Texture:
        glGenTextures(1, &m_Depth);
        glBindTexture(GL_TEXTURE_2D, m_Depth);

        if (m_Info.depth.hasStencil) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Info.resolution.x, m_Info.resolution.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_Depth, 0);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_Info.resolution.x, m_Info.resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Depth, 0);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        break;

    case DepthAttachmentInfo::Storage::Renderbuffer:
        glGenRenderbuffers(1, &m_Depth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_Depth);

        if (m_Info.depth.hasStencil) {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Info.resolution.x, m_Info.resolution.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Depth);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, m_Info.resolution.x, m_Info.resolution.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_Depth);
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        break;

    case DepthAttachmentInfo::Storage::None:
        break;
    }

    PBZ_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "[Framebuffer] Incomplete Framebuffer.");

    unbind();

    return true;
}

bool Framebuffer::destroy() {
    if (m_Framebuffer == 0) {
        Logger::WARNING("[Framebuffer] Trying to delete a destroyed framebuffer");
        return false;
    }

    for (std::size_t i = 0; i < m_Colors.size(); i++) {
        switch (m_Info.colors[i].storage) {
        case ColorAttachmentInfo::Storage::Texture:
            glDeleteTextures(1, &m_Colors[i]);
            break;

        case ColorAttachmentInfo::Storage::Renderbuffer:
            glDeleteRenderbuffers(1, &m_Colors[i]);
            break;

        case ColorAttachmentInfo::Storage::None:
            break;
        }
    }

    m_Colors.clear();

    switch (m_Info.depth.storage) {
    case DepthAttachmentInfo::Storage::Texture:
        glDeleteTextures(1, &m_Depth);
        break;

    case DepthAttachmentInfo::Storage::Renderbuffer:
        glDeleteRenderbuffers(1, &m_Depth);
        break;

    case DepthAttachmentInfo::Storage::None:
        break;
    }

    glDeleteFramebuffers(1, &m_Framebuffer);
    m_Framebuffer = 0;

    return true;
}

void Framebuffer::resize(const glm::ivec2 &resolution) {
    m_Info.resolution = resolution;

    bind();

    for (std::size_t i = 0; i < m_Colors.size(); i++) {
        switch (m_Info.colors[i].storage) {
        case ColorAttachmentInfo::Storage::Texture:
            glBindTexture(GL_TEXTURE_2D, m_Colors[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            break;

        case ColorAttachmentInfo::Storage::Renderbuffer:
            glBindRenderbuffer(GL_RENDERBUFFER, m_Colors[i]);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, resolution.x, resolution.y);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            break;

        case ColorAttachmentInfo::Storage::None:
            break;
        }
    }

    switch (m_Info.depth.storage) {
    case DepthAttachmentInfo::Storage::Texture:
        glBindTexture(GL_TEXTURE_2D, m_Depth);

        if (m_Info.depth.hasStencil) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, resolution.x, resolution.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, resolution.x, resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        break;

    case DepthAttachmentInfo::Storage::Renderbuffer:
        glBindRenderbuffer(GL_RENDERBUFFER, m_Depth);
        if (m_Info.depth.hasStencil) {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, resolution.x, resolution.y);
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        break;

    case DepthAttachmentInfo::Storage::None:
        break;
    }

    PBZ_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "[Framebuffer] Incomplete Framebuffer after resize.");

    glViewport(0, 0, resolution.x, resolution.y);

    unbind();
}

void Framebuffer::clear() const {
    bind();
    glClearColor(m_Info.colorClear.r, m_Info.colorClear.g, m_Info.colorClear.b, m_Info.colorClear.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    unbind();
}

void Framebuffer::bind() const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to bind an incomplete framebuffer.");
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::tuple<ColorAttachmentInfo, GLuint> Framebuffer::getColor(std::size_t index) const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to get from an incomplete framebuffer.");
    if (index >= m_Colors.size()) {
        return std::make_tuple(ColorAttachmentInfo{}, 0);
    }

    return {m_Info.colors[index], m_Colors[index]};
}

std::tuple<DepthAttachmentInfo, GLuint> Framebuffer::getDepth() const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to get from an incomplete framebuffer.");
    return {m_Info.depth, m_Depth};
}

const FramebufferInfo &Framebuffer::getInfo() const {
    return m_Info;
}

bool Framebuffer::bindOutputTexture(GLint unit) const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to get from an incomplete framebuffer.");
    GLuint texture;

    switch (m_Info.output.type) {
    case OutputAttachmentInfo::Type::Color:
        texture = m_Colors[m_Info.output.colorIndex];
        break;

    case OutputAttachmentInfo::Type::Depth:
        texture = m_Depth;
        break;
    }

    GL::TextureUnits::activate();
    glBindTexture(GL_TEXTURE_2D, texture);
    return true;
}

bool Framebuffer::unbindOutputTexture() const {
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

} // namespace Physbuzz
