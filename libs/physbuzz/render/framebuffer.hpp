#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

struct ColorAttachmentInfo {
    enum class Storage {
        None,
        Texture,
        Renderbuffer,
    } storage = Storage::None;

    bool isDrawn = false;
};

struct DepthAttachmentInfo {
    enum class Storage {
        None,
        Texture,
        Renderbuffer,
    } storage = Storage::None;

    bool hasStencil = false;
};

struct OutputAttachmentInfo {
    enum class Type {
        Color,
        Depth,
    } type = Type::Color;
    std::size_t colorIndex = 0;
};

struct FramebufferInfo {
    glm::ivec2 resolution = {1280, 720};
    glm::vec4 colorClear = {0.0f, 0.0f, 0.0f, 0.0f};

    std::vector<ColorAttachmentInfo> colors;
    DepthAttachmentInfo depth;
    OutputAttachmentInfo output = {};
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

    bool bindOutputTexture(GLint unit) const;
    bool unbindOutputTexture() const;

    std::tuple<ColorAttachmentInfo, GLuint> getColor(std::size_t index) const;
    std::tuple<DepthAttachmentInfo, GLuint> getDepth() const;

    const FramebufferInfo &getInfo() const;

  private:
    GLuint m_Framebuffer = 0;

    std::vector<GLuint> m_Colors;
    GLuint m_Depth;

    FramebufferInfo m_Info;
};

} // namespace Physbuzz
