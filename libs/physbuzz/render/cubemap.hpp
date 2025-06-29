#pragma once

#include "../io/image.hpp"
#include "../resources/defines.hpp"
#include <glad/gl.h>

namespace Physbuzz {

struct CubemapInfo {
    ImageInfo right;
    ImageInfo left;
    ImageInfo top;
    ImageInfo bottom;
    ImageInfo back;
    ImageInfo front;
};

class Cubemap : public ResourceTag {
  public:
    Cubemap(const CubemapInfo &info);
    ~Cubemap();

    bool build();
    bool destroy();

    bool bind() const;
    bool unbind() const;

  private:
    CubemapInfo m_Info;
    GLuint m_Texture = 0;

    bool loadImage(ImageInfo &imageInfo, GLenum target);
};

} // namespace Physbuzz
