#pragma once

#include "../io/image.hpp"
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

class CubemapResource {
  public:
    CubemapResource(const CubemapInfo &info);
    ~CubemapResource();

    bool build();
    bool destroy();

    bool bind(bool depthMask = false) const;
    bool unbind(bool depthMask = false) const;

    const GLint &getUnit() const;

  private:
    CubemapInfo m_Info;
    GLuint m_Texture = 0;
    GLint m_Unit = 0;

    bool loadImage(ImageInfo &imageInfo, GLenum target);
};

} // namespace Physbuzz
