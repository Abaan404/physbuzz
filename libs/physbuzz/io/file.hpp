#pragma once

#include <filesystem>

namespace Physbuzz {

struct FileInfo {
    std::filesystem::path path;
};

class FileResource {
  public:
    FileResource(const FileInfo &file);
    ~FileResource();

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
