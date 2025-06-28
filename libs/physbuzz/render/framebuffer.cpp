#include "framebuffer.hpp"

#include "../debug/logging.hpp"

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
    case FramebufferInfo::Type::Color:
        if (m_Info.output.colorIndex >= m_Info.colors.size()) {
            Logger::ERROR(std::format("[Framebuffer] Invalid output color index {}", m_Info.output.colorIndex));
            return false;
        }

        if (!m_Info.colors[m_Info.output.colorIndex].isDrawn ||
            m_Info.colors[m_Info.output.colorIndex].storage == FramebufferInfo::Storage::Renderbuffer ||
            m_Info.colors[m_Info.output.colorIndex].storage == FramebufferInfo::Storage::None) {
            Logger::ERROR(std::format("[Framebuffer] output color index cannot be drawn {}", m_Info.output.colorIndex));
            return false;
        }
        break;

    case FramebufferInfo::Type::Depth:
        if (m_Info.depth.storage == FramebufferInfo::Storage::Renderbuffer ||
            m_Info.depth.storage == FramebufferInfo::Storage::None) {
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
        case FramebufferInfo::Storage::Texture2D:
            glGenTextures(1, &color);
            glBindTexture(GL_TEXTURE_2D, color);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Info.resolution.x, m_Info.resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_TEXTURE_2D, color, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            break;

        case FramebufferInfo::Storage::Cubemap:
            glGenTextures(1, &color);
            glBindTexture(GL_TEXTURE_CUBE_MAP, color);

            for (std::size_t i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, m_Info.resolution.x, m_Info.resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, color, 0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            break;

        case FramebufferInfo::Storage::Renderbuffer:
            glGenRenderbuffers(1, &color);
            glBindRenderbuffer(GL_RENDERBUFFER, color);

            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_Info.resolution.x, m_Info.resolution.y);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_RENDERBUFFER, color);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            break;

        case FramebufferInfo::Storage::None:
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
    case FramebufferInfo::Storage::Texture2D:
        glGenTextures(1, &m_Depth);
        glBindTexture(GL_TEXTURE_2D, m_Depth);

        if (m_Info.depth.hasStencil) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Info.resolution.x, m_Info.resolution.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_Depth, 0);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_Info.resolution.x, m_Info.resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Depth, 0);
        }

        {
            float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        break;

    case FramebufferInfo::Storage::Cubemap:
        glGenTextures(1, &m_Depth);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_Depth);

        for (std::size_t i = 0; i < 6; ++i) {
            if (m_Info.depth.hasStencil) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH24_STENCIL8, m_Info.resolution.x, m_Info.resolution.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            } else {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, m_Info.resolution.x, m_Info.resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            }
        }

        if (m_Info.depth.hasStencil) {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_Depth, 0);
        } else {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_Depth, 0);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        break;

    case FramebufferInfo::Storage::Renderbuffer:
        glGenRenderbuffers(1, &m_Depth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_Depth);

        if (m_Info.depth.hasStencil) {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Info.resolution.x, m_Info.resolution.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Depth);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, m_Info.resolution.x, m_Info.resolution.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_Depth);
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        break;

    case FramebufferInfo::Storage::None:
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
        case FramebufferInfo::Storage::Texture2D:
        case FramebufferInfo::Storage::Cubemap:
            glDeleteTextures(1, &m_Colors[i]);
            break;

        case FramebufferInfo::Storage::Renderbuffer:
            glDeleteRenderbuffers(1, &m_Colors[i]);
            break;

        case FramebufferInfo::Storage::None:
            break;
        }
    }

    m_Colors.clear();

    switch (m_Info.depth.storage) {
    case FramebufferInfo::Storage::Texture2D:
    case FramebufferInfo::Storage::Cubemap:
        glDeleteTextures(1, &m_Depth);
        break;

    case FramebufferInfo::Storage::Renderbuffer:
        glDeleteRenderbuffers(1, &m_Depth);
        break;

    case FramebufferInfo::Storage::None:
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
        case FramebufferInfo::Storage::Texture2D:
            glBindTexture(GL_TEXTURE_2D, m_Colors[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            break;

        case FramebufferInfo::Storage::Cubemap:
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_Colors[i]);
            for (std::size_t j = 0; j < 6; ++j) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            }
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            break;

        case FramebufferInfo::Storage::Renderbuffer:
            glBindRenderbuffer(GL_RENDERBUFFER, m_Colors[i]);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, resolution.x, resolution.y);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            break;

        case FramebufferInfo::Storage::None:
            break;
        }
    }

    switch (m_Info.depth.storage) {
    case FramebufferInfo::Storage::Texture2D:
        glBindTexture(GL_TEXTURE_2D, m_Depth);

        if (m_Info.depth.hasStencil) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, resolution.x, resolution.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, resolution.x, resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        break;

    case FramebufferInfo::Storage::Cubemap:
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_Depth);
        for (std::size_t i = 0; i < 6; ++i) {
            if (m_Info.depth.hasStencil) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH24_STENCIL8, resolution.x, resolution.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            } else {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, resolution.x, resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            }
        }
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        break;

    case FramebufferInfo::Storage::Renderbuffer:
        glBindRenderbuffer(GL_RENDERBUFFER, m_Depth);
        if (m_Info.depth.hasStencil) {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, resolution.x, resolution.y);
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        break;

    case FramebufferInfo::Storage::None:
        break;
    }

    PBZ_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "[Framebuffer] Incomplete Framebuffer after resize.");

    unbind();
}

void Framebuffer::clear() const {
    GLenum clear = 0;

    if (!m_Colors.empty()) {
        clear |= GL_COLOR_BUFFER_BIT;
    }

    if (m_Info.depth.storage != FramebufferInfo::Storage::None) {
        clear |= GL_DEPTH_BUFFER_BIT;
        if (m_Info.depth.hasStencil) {
            clear |= GL_STENCIL_BUFFER_BIT;
        }
    }

    bind();
    glClearColor(m_Info.colorClear.r, m_Info.colorClear.g, m_Info.colorClear.b, m_Info.colorClear.a);
    glClear(clear);
    unbind();
}

void Framebuffer::bind() const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to bind an incomplete framebuffer.");
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
    glViewport(0, 0, m_Info.resolution.x, m_Info.resolution.y);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

const FramebufferInfo &Framebuffer::getInfo() const {
    return m_Info;
}

bool Framebuffer::bindOutputTexture() const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to bind output from an incomplete framebuffer.");
    FramebufferInfo::Storage storage = FramebufferInfo::Storage::None;
    GLuint texture;

    switch (m_Info.output.type) {
    case FramebufferInfo::Type::Color:
        storage = m_Info.colors[m_Info.output.colorIndex].storage;
        texture = m_Colors[m_Info.output.colorIndex];
        break;

    case FramebufferInfo::Type::Depth:
        storage = m_Info.depth.storage;
        texture = m_Depth;
        break;
    }

    switch (storage) {
    case FramebufferInfo::Storage::Texture2D:
        glBindTexture(GL_TEXTURE_2D, texture);
        break;

    case FramebufferInfo::Storage::Cubemap:
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        break;

    case FramebufferInfo::Storage::Renderbuffer:
    case FramebufferInfo::Storage::None:
        Logger::ERROR("[Framebuffer] Cannot bind output attachment as a texture.");
        return false;
    }

    return true;
}

bool Framebuffer::unbindOutputTexture() const {
    PBZ_ASSERT(m_Framebuffer != 0, "[Framebuffer] trying to unbind output from an incomplete framebuffer.");
    FramebufferInfo::Storage storage = FramebufferInfo::Storage::None;

    switch (m_Info.output.type) {
    case FramebufferInfo::Type::Color:
        storage = m_Info.colors[m_Info.output.colorIndex].storage;
        break;

    case FramebufferInfo::Type::Depth:
        storage = m_Info.depth.storage;
        break;
    }

    switch (storage) {
    case FramebufferInfo::Storage::Texture2D:
        glBindTexture(GL_TEXTURE_2D, 0);
        break;

    case FramebufferInfo::Storage::Cubemap:
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        break;

    case FramebufferInfo::Storage::Renderbuffer:
    case FramebufferInfo::Storage::None:
        break;
    }

    return true;
}

} // namespace Physbuzz
