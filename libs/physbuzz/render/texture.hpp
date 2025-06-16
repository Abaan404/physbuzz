#pragma once

#include "../io/image.hpp"
#include <glad/gl.h>

namespace Physbuzz {

enum class TextureType;

struct Texture2DInfo {
    ImageInfo image;
    TextureType type;
};

class Texture2DResource {
  public:
    Texture2DResource(const Texture2DInfo &texture2D);
    ~Texture2DResource();

    bool build();
    bool destroy();

    bool bind() const;
    bool unbind() const;

    TextureType getType() const;

  private:
    Texture2DInfo m_Info;
    GLuint m_Texture;
};

} // namespace Physbuzz
