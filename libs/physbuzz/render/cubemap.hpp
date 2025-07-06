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

    GLint activate(GLint unit = -1) const;

    const Info &getInfo() const;

  private:
    Info m_Info;
    GLuint m_Texture = 0;
};

template <>
struct IsResource<Cubemap> : std::true_type {};

} // namespace Physbuzz
