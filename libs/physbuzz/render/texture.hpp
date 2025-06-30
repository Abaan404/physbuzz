#pragma once

#include "../io/image.hpp"
#include "../resources/defines.hpp"
#include <glad/gl.h>

namespace Physbuzz {

enum class TextureType;

class Texture2D {
  public:
    struct Info {
        ImageFile::Info image;
        TextureType type;
    };

    Texture2D(const Info &texture2D);
    ~Texture2D();

    bool build();
    bool destroy();

    bool bind() const;
    bool unbind() const;

    TextureType getType() const;

  private:
    Info m_Info;
    GLuint m_Texture;
};

template <>
struct IsResource<Texture2D> : std::true_type {};

} // namespace Physbuzz
