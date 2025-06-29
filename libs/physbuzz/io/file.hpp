#pragma once

#include "../resources/defines.hpp"
#include <filesystem>

namespace Physbuzz {

struct FileInfo {
    std::filesystem::path path;
};

class File : public ResourceTag {
  public:
    File(const FileInfo &file);
    ~File();

    bool build();
    bool destroy();

    bool read();
    bool write();

    const std::streampos &getSize() const;
    const std::filesystem::path &getPath() const;

    std::string buffer;

  private:
    FileInfo m_Info;
    std::streampos m_Size = 0;
};

} // namespace Physbuzz
