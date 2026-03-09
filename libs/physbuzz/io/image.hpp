#pragma once

#include "file.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

class ImageFile {
  public:
    struct Info {
        std::vector<File::Info> files;
        bool flipVertically = false;
    };

    struct Data {
        std::vector<std::byte> image;

        struct {
            glm::uvec2 resolution = {0, 0};
            std::size_t size = 0;
        } meta;
    };

    ImageFile(const Info &image);
    ~ImageFile();

    bool read();
    bool write(const Info &info, const Data &data);

    bool readMeta();

    const Data &getData() const;
    const Info &getInfo() const;

  private:
    Data m_Data;
    Info m_Info;
};

template <>
struct IsResource<ImageFile> : std::true_type {};

} // namespace Physbuzz
