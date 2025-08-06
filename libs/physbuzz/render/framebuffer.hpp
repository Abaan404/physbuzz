#pragma once

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

    enum Mask {
        Color,
        Depth,
        Stencil,
    };

    enum class Type {
        Color,
        Depth,
    };

    struct Rect {
        glm::ivec2 p1;
        glm::ivec2 p2;
    };

    struct ColorAttachment {
        Storage storage = Storage::None;
        bool isDrawn = false;
    };

    struct DepthAttachment {
        Storage storage = Storage::None;
        bool hasStencil = false;
    };

    struct Info {
        glm::ivec2 resolution = {1, 1};
        struct {
            glm::vec4 color = {0.0f, 0.0f, 0.0f, 0.0f};
            float depth = 1.0f;
            float stencil = 1.0f;
        } clear;

        std::vector<ColorAttachment> colors;
        DepthAttachment depth;
    };

    Framebuffer(const Info &info);

    bool build();
    bool destroy();

    void bind() const;
    void unbind() const;

    bool resize(const glm::ivec2 &resolution);
    void clear() const;
    void blit(const Framebuffer &framebuffer, Rect from, Rect to, Mask mask = Mask::Color) const;

    std::int32_t activate(Type type, std::size_t colorIndex = 0, std::int32_t unit = -1) const;

    const Info &getInfo() const;
    std::uint32_t getImGuiTextureHandle(Type type, std::size_t colorIndex = 0) const;

  private:
    std::uint32_t m_Framebuffer = 0;

    std::vector<std::uint32_t> m_Colors;
    std::uint32_t m_Depth = 0;

    Info m_Info;
};

} // namespace Physbuzz
