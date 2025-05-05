#pragma once

#include "../resources/manager.hpp"
#include "physbuzz/resources/image.hpp"

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
    GLuint m_Texture;
    GLint m_Unit;

    std::uint8_t loadImage(ImageInfo &imageInfo, GLenum target);

    // to access m_Unit in container
    template <ResourceType T>
    friend class ResourceContainer;
};

template <>
bool ResourceContainer<CubemapResource>::insert(const std::string &identifier, CubemapResource &&resource);

template <>
bool ResourceContainer<CubemapResource>::erase(const std::string &identifier);

} // namespace Physbuzz
