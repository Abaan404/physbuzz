#pragma once

#include "file.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

struct ImageInfo {
    FileInfo file;
};

class ImageResource {
  public:
    ImageResource(const ImageInfo &image);
    ~ImageResource();

    bool build();
    bool destroy();

    bool read();
    bool write();

    const int &getChannels() const;
    const glm::ivec2 &getResolution() const;

    std::uint8_t *buffer = nullptr;

  private:
    ImageInfo m_Info;
    glm::ivec2 m_Resolution;
    int m_Channels = 0;
};

} // namespace Physbuzz
