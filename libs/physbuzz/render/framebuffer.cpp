#include "framebuffer.hpp"

#include "../debug/logging.hpp"
#include "gl/units.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace Physbuzz {

Framebuffer::Framebuffer(const Info &info)
    : m_Info(info) {}

bool Framebuffer::build() {
    if (m_Framebuffer != 0) {
        Logger::WARNING("[Framebuffer] Trying to build a built framebuffer");
        return true;
    }

    if (m_Info.depth.storage != Storage::None && m_Depth != 0) {
        Logger::WARNING("[Framebuffer] Trying to build a built depth buffer.");
        return true;
    }

    GLint maxColors = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColors);

    if (m_Info.colors.size() > maxColors) {
        Logger::ERROR("[Framebuffer] Too many color attachments, max supported is {}", maxColors);
        return false;
    }

    glCreateFramebuffers(1, &m_Framebuffer);

    GLuint attachmentIndex = 0;

    std::vector<GLenum> drawBuffers;
    for (const auto &attachment : m_Info.colors) {
        GLuint color = 0;

        switch (attachment.storage) {
        case Storage::Texture2D:
            glCreateTextures(GL_TEXTURE_2D, 1, &color);

            glTextureParameteri(color, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(color, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTextureStorage2D(color, 1, GL_RGBA16F, m_Info.resolution.x, m_Info.resolution.y);
            glTextureSubImage2D(color, 0, 0, 0, m_Info.resolution.x, m_Info.resolution.y, GL_RGBA, GL_FLOAT, nullptr);
            glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0 + attachmentIndex, color, 0);
            break;

        case Storage::Cubemap:
            glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &color);

            glTextureParameteri(color, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(color, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(color, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(color, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(color, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glTextureStorage2D(color, 1, GL_RGBA16F, m_Info.resolution.x, m_Info.resolution.y);
            for (std::size_t i = 0; i < 6; ++i) {
                glTextureSubImage3D(color, 0, 0, 0, i, m_Info.resolution.x, m_Info.resolution.y, 1, GL_RGBA, GL_FLOAT, nullptr);
            }

            glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0 + attachmentIndex, color, 0);
            break;

        case Storage::Renderbuffer:
            glCreateRenderbuffers(1, &color);
            glNamedRenderbufferStorage(color, GL_RGBA16F, m_Info.resolution.x, m_Info.resolution.y);
            glNamedFramebufferRenderbuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_RENDERBUFFER, color);
            break;

        case Storage::None:
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

    if (!drawBuffers.empty()) {
        glNamedFramebufferDrawBuffers(m_Framebuffer, drawBuffers.size(), drawBuffers.data());
    } else {
        glNamedFramebufferDrawBuffer(m_Framebuffer, GL_NONE);
        glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
    }

    switch (m_Info.depth.storage) {
    case Storage::Texture2D:
        glCreateTextures(GL_TEXTURE_2D, 1, &m_Depth);

        {
            float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
            glTextureParameterfv(m_Depth, GL_TEXTURE_BORDER_COLOR, borderColor);
        }
        glTextureParameteri(m_Depth, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_Depth, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(m_Depth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_Depth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (m_Info.depth.hasStencil) {
            glTextureStorage2D(m_Depth, 1, GL_DEPTH24_STENCIL8, m_Info.resolution.x, m_Info.resolution.y);
            glTextureSubImage2D(m_Depth, 0, 0, 0, m_Info.resolution.x, m_Info.resolution.y, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_Depth, 0);
        } else {
            glTextureStorage2D(m_Depth, 1, GL_DEPTH_COMPONENT32F, m_Info.resolution.x, m_Info.resolution.y);
            glTextureSubImage2D(m_Depth, 0, 0, 0, m_Info.resolution.x, m_Info.resolution.y, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_Depth, 0);
        }

        break;

    case Storage::Cubemap:
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Depth);

        glTextureParameteri(m_Depth, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(m_Depth, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_Depth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_Depth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_Depth, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        if (m_Info.depth.hasStencil) {
            glTextureStorage2D(m_Depth, 1, GL_DEPTH24_STENCIL8, m_Info.resolution.x, m_Info.resolution.y);
            for (std::size_t i = 0; i < 6; ++i) {
                glTextureSubImage3D(m_Depth, 0, 0, 0, i, m_Info.resolution.x, m_Info.resolution.y, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            }

            glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_Depth, 0);
        } else {
            glTextureStorage2D(m_Depth, 1, GL_DEPTH_COMPONENT32F, m_Info.resolution.x, m_Info.resolution.y);
            for (std::size_t i = 0; i < 6; ++i) {
                glTextureSubImage3D(m_Depth, 0, 0, 0, i, m_Info.resolution.x, m_Info.resolution.y, 1, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            }

            glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_Depth, 0);
        }

        break;

    case Storage::Renderbuffer:
        glCreateRenderbuffers(1, &m_Depth);

        if (m_Info.depth.hasStencil) {
            glNamedRenderbufferStorage(m_Depth, GL_DEPTH24_STENCIL8, m_Info.resolution.x, m_Info.resolution.y);
            glNamedFramebufferRenderbuffer(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Depth);
        } else {
            glNamedRenderbufferStorage(m_Depth, GL_DEPTH_COMPONENT32F, m_Info.resolution.x, m_Info.resolution.y);
            glNamedFramebufferRenderbuffer(m_Framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_Depth);
        }

        break;

    case Storage::None:
        break;
    }

    PBZ_ASSERT(glCheckNamedFramebufferStatus(m_Framebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "[Framebuffer] Incomplete Framebuffer.");

    return true;
}

bool Framebuffer::destroy() {
    if (m_Framebuffer == 0) {
        Logger::WARNING("[Framebuffer] Trying to delete a destructed framebuffer");
        return true;
    }

    for (std::size_t i = 0; i < m_Colors.size(); i++) {
        switch (m_Info.colors[i].storage) {
        case Storage::Texture2D:
        case Storage::Cubemap:
            glDeleteTextures(1, &m_Colors[i]);
            break;

        case Storage::Renderbuffer:
            glDeleteRenderbuffers(1, &m_Colors[i]);
            break;

        case Storage::None:
            break;
        }
    }

    m_Colors.clear();

    switch (m_Info.depth.storage) {
    case Storage::Texture2D:
    case Storage::Cubemap:
        glDeleteTextures(1, &m_Depth);
        m_Depth = 0;
        break;

    case Storage::Renderbuffer:
        glDeleteRenderbuffers(1, &m_Depth);
        m_Depth = 0;
        break;

    case Storage::None:
        break;
    }

    glDeleteFramebuffers(1, &m_Framebuffer);
    m_Framebuffer = 0;

    return true;
}

bool Framebuffer::resize(const glm::ivec2 &resolution) {
    m_Info.resolution = resolution;

    if (!(destroy() && build())) {
        Logger::ERROR("[Framebuffer] Could not resize framebuffer.");
        return false;
    }

    return true;
}

void Framebuffer::clear() const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to clear an incomplete framebuffer.");

    for (std::size_t i = 0; i < m_Info.colors.size(); i++) {
        if (m_Info.colors[i].isDrawn) {
            glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, i, glm::value_ptr(m_Info.clear.color));
        }
    }

    if (m_Info.depth.storage != Storage::None) {
        if (m_Info.depth.hasStencil) {
            glClearNamedFramebufferfi(m_Framebuffer, GL_DEPTH_STENCIL, 0, m_Info.clear.depth, m_Info.clear.stencil);
        } else {
            glClearNamedFramebufferfv(m_Framebuffer, GL_DEPTH, 0, &m_Info.clear.depth);
        }
    }
}

void Framebuffer::blit(const Framebuffer &framebuffer, Rect from, Rect to, Mask mask) const {
    glBlitNamedFramebuffer(framebuffer.m_Framebuffer, m_Framebuffer, from.p1.x, from.p1.y, from.p2.x, from.p2.y, to.p1.x, to.p1.y, to.p2.x, to.p2.y, mask, GL_NEAREST);
}

void Framebuffer::bind() const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to bind an incomplete framebuffer.");
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
    glViewport(0, 0, m_Info.resolution.x, m_Info.resolution.y);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLint Framebuffer::activate(Type type, std::size_t colorIndex, GLint unit) const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to activate output from an incomplete framebuffer.");

    switch (type) {
    case Type::Color:
        if (colorIndex >= m_Info.colors.size()) {
            Logger::ERROR("[Framebuffer] Invalid output color index {}", colorIndex);
            return false;
        }

        if (!m_Info.colors[colorIndex].isDrawn ||
            m_Info.colors[colorIndex].storage == Storage::Renderbuffer ||
            m_Info.colors[colorIndex].storage == Storage::None) {
            Logger::ERROR("[Framebuffer] output color index cannot be drawn {}", colorIndex);
            return false;
        }
        break;

    case Type::Depth:
        if (m_Info.depth.storage == Storage::Renderbuffer ||
            m_Info.depth.storage == Storage::None) {
            Logger::ERROR("[Framebuffer] output depth cannot be drawn");
            return false;
        }
        break;
    }

    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to activate output from an incomplete framebuffer.");
    GLuint texture;

    switch (type) {
    case Type::Color:
        texture = m_Colors[colorIndex];
        break;

    case Type::Depth:
        texture = m_Depth;
        break;
    }

    return GL::detail::TextureUnits::activate(texture, unit);
}

const Framebuffer::Info &Framebuffer::getInfo() const {
    return m_Info;
}

const std::vector<GLuint> &Framebuffer::getColors() const {
    return m_Colors;
}

const GLuint &Framebuffer::getDepth() const {
    return m_Depth;
}

} // namespace Physbuzz
