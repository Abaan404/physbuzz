#pragma once

#include <string>
#include <vector>

namespace Physbuzz {

struct FileInfo {
    std::string path = "";
};

class FileResource {
  public:
    FileResource(const FileInfo &file);
    ~FileResource();

    void build();
    void destroy();

    void read();
    void write();

    const std::streampos &getSize() const;

    std::vector<char> buffer;

  private:
    FileInfo m_Info;
    std::streampos m_Size = 0;
};

} // namespace Physbuzz
