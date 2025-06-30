#pragma once

#include "file.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

class ImageFile {
  public:
    struct Info {
        File::Info file;
        bool flipVertically = false;
    };

    ImageFile(const Info &image);
    ~ImageFile();

    bool build();
    bool destroy();

    bool read();
    bool write();

    const int &getChannels() const;
    const glm::ivec2 &getResolution() const;

    std::uint8_t *buffer = nullptr;

  private:
    Info m_Info;
    glm::ivec2 m_Resolution;
    int m_Channels = 0;
};

template <>
struct IsResource<ImageFile> : std::true_type {};

} // namespace Physbuzz
