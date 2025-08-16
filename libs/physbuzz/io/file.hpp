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
        std::vector<std::uint8_t> buffer;
    };

    File(const Info &file);

    bool build();
    bool destroy();

    bool read();
    bool write();

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Data m_Data;
    Info m_Info;
};

template <>
struct IsResource<File> : std::true_type {};

} // namespace Physbuzz
