#pragma once

#include "../io/image.hpp"
#include "../resources/defines.hpp"
#include <glad/gl.h>

namespace Physbuzz {

class Cubemap {
  public:
    struct Info {
        ImageFile::Info right;
        ImageFile::Info left;
        ImageFile::Info top;
        ImageFile::Info bottom;
        ImageFile::Info back;
        ImageFile::Info front;
    };

    Cubemap(const Info &info);
    ~Cubemap();

    bool build();
    bool destroy();

    bool bind() const;
    bool unbind() const;

  private:
    Info m_Info;
    GLuint m_Texture = 0;

    bool loadImage(ImageFile::Info &imageInfo, GLenum target);
};

template <>
struct IsResource<Cubemap> : std::true_type {};

} // namespace Physbuzz
