#pragma once

#include "../resources/defines.hpp"
#include <filesystem>

namespace Physbuzz {

class File : public ResourceTag {
  public:
    struct Info {
        std::filesystem::path path;
    };

    File(const Info &file);
    ~File();

    bool build();
    bool destroy();

    bool read();
    bool write();

    const std::streampos &getSize() const;
    const std::filesystem::path &getPath() const;

    std::string buffer;

  private:
    Info m_Info;
    std::streampos m_Size = 0;
};

} // namespace Physbuzz
