#pragma once

#include "file.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

class ImageFile {
  public:
    struct Info {
        File::Info file;
        bool flipVertically = false;
    };

    struct Data {
        std::vector<std::byte> image;
        glm::ivec2 resolution = {0, 0};
    };

    ImageFile(const Info &image);
    ~ImageFile();

    bool build();
    bool destroy();

    bool read();
    bool write(const Info &info, const Data &data);

    const Data &getData() const;
    const Info &getInfo() const;

  private:
    Data m_Data;
    Info m_Info;
};

template <>
struct IsResource<ImageFile> : std::true_type {};

} // namespace Physbuzz
