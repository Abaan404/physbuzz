#pragma once

#include "file.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

class ImageFile {
  public:
    struct Info {
        std::vector<File::Info> files;
        bool flipVertically = false;
    };

    struct Data {
        std::vector<std::byte> image;

        struct Meta {
            glm::uvec2 resolution = {0, 0};
            std::size_t size = 0;
            bool hdr = false;
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

    bool readExr(const std::filesystem::path &path, std::size_t offset);
    bool readStb(const std::filesystem::path &path, std::size_t offset);

    bool writeExr(const std::filesystem::path &path, std::size_t offset, const Data &data);
    bool writeStb(const std::filesystem::path &path, std::size_t offset, const Data &data);

    std::optional<Data::Meta> readExrMeta(const std::filesystem::path &path) const;
    std::optional<Data::Meta> readStbMeta(const std::filesystem::path &path) const;
};

template <>
struct IsResource<ImageFile> : std::true_type {};

} // namespace Physbuzz
