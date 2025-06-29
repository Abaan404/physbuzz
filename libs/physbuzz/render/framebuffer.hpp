#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

class Framebuffer {
  public:
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

    struct Info {
        glm::ivec2 resolution = {1280, 720};
        glm::vec4 colorClear = {0.0f, 0.0f, 0.0f, 0.0f};

        std::vector<ColorAttachment> colors;
        DepthAttachment depth;
        OutputAttachment output;
    };

    Framebuffer(const Info &info);

    bool build();
    bool destroy();

    void bind() const;
    void unbind() const;

    void resize(const glm::ivec2 &resolution);
    void clear() const;

    bool bindOutputTexture() const;
    bool unbindOutputTexture() const;

    const Info &getInfo() const;

  private:
    GLuint m_Framebuffer = 0;

    std::vector<GLuint> m_Colors;
    GLuint m_Depth;

    Info m_Info;
};

} // namespace Physbuzz
