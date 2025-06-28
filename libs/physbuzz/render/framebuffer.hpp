#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

struct FramebufferInfo {
    enum class Storage {
        Texture2D,
        Cubemap,
        Renderbuffer,
        None,
    };

    enum class Type {
        Color,
        Depth,
    };

    struct ColorAttachment {
        Storage storage = Storage::None;
        bool isDrawn = false;
    };

    struct DepthAttachment {
        Storage storage = Storage::None;
        bool hasStencil = false;
    };

    struct OutputAttachment {
        Type type = Type::Color;
        std::size_t colorIndex = 0;
    };

    glm::ivec2 resolution = {1280, 720};
    glm::vec4 colorClear = {0.0f, 0.0f, 0.0f, 0.0f};

    std::vector<ColorAttachment> colors = {};
    DepthAttachment depth = {};
    OutputAttachment output = {};
};

class Framebuffer {
  public:
    Framebuffer(const FramebufferInfo &info);

    bool build();
    bool destroy();

    void bind() const;
    void unbind() const;

    void resize(const glm::ivec2 &resolution);
    void clear() const;

    bool bindOutputTexture() const;
    bool unbindOutputTexture() const;

    const FramebufferInfo &getInfo() const;

  private:
    GLuint m_Framebuffer = 0;

    std::vector<GLuint> m_Colors;
    GLuint m_Depth;

    FramebufferInfo m_Info;
};

} // namespace Physbuzz
