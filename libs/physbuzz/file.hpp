#pragma once

#include <glm/glm.hpp>
#include <iosfwd>
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

struct ImageInfo {
    FileInfo file;
};

class ImageResource {
  public:
    ImageResource(const ImageInfo &image);
    ~ImageResource();

    void build();
    void destroy();

    void read();
    void write();

    const int &getChannels() const;
    const glm::ivec2 &getResolution() const;

    unsigned char *buffer = nullptr;

  private:
    ImageInfo m_Info;
    glm::ivec2 m_Resolution;
    int m_Channels = 0;
};

} // namespace Physbuzz
