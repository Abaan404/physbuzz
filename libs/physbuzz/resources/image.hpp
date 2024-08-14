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

    void build();
    void destroy();

    void read();
    void write();

    const int &getChannels() const;
    const glm::ivec2 &getResolution() const;

    unsigned char *buffer = nullptr;

  private:
    ImageInfo m_Info;
    glm::ivec2 m_Resolution;
    int m_Channels = 0;
};

} // namespace Physbuzz
