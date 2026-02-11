#pragma once

#include "../resources/defines.hpp"
#include <filesystem>

namespace Physbuzz {

class File {
  public:
    struct Info {
        std::filesystem::path path;
    };

    struct Data {
        std::vector<std::byte> buffer;
    };

    File(const Info &file);

    bool read();
    bool write(const Data &data);

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Data m_Data;
    Info m_Info;
};

template <>
struct IsResource<File> : std::true_type {};

} // namespace Physbuzz
