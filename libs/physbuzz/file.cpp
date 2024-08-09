#include "file.hpp"

#include "logging.hpp"
#include <format>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace Physbuzz {

FileResource::FileResource(const FileInfo &file)
    : m_Info(file) {}

FileResource::~FileResource() {}

void FileResource::build() {}

void FileResource::destroy() {}

void FileResource::read() {
    std::ifstream stream = std::ifstream(m_Info.path, std::ios::in | std::ios::binary);

    if (!stream.is_open()) {
        Logger::WARNING(std::format("[File] Failed to open file: {}", m_Info.path));
        return;
    }

    stream.seekg(0, std::ios::end);
    m_Size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    buffer.resize(m_Size);
    stream.read(buffer.data(), buffer.size());
    buffer.push_back('\0');

    if (stream.fail()) {
        Logger::WARNING(std::format("[File] Failed to read file: {}", m_Info.path));
        return;
    }

    stream.close();
}

void FileResource::write() {
    std::ofstream stream = std::ofstream(m_Info.path, std::ios::out | std::ios::binary);

    if (!stream.is_open()) {
        Logger::WARNING(std::format("[File] Failed to open file: {}", m_Info.path));
        return;
    }

    stream.write(buffer.data(), buffer.size());

    if (stream.fail()) {
        Logger::WARNING(std::format("[File] Failed to write file: {}", m_Info.path));
        return;
    }

    stream.close();
    if (stream.bad()) {
        Logger::WARNING(std::format("[File] Failed to close file: {}", m_Info.path));
        return;
    }
}

const std::streampos &FileResource::getSize() const {
    return m_Size;
}

ImageResource::ImageResource(const ImageInfo &image)
    : m_Info(image) {}

ImageResource::~ImageResource() {}

void ImageResource::build() {}

void ImageResource::destroy() {
    stbi_image_free(buffer);
}

void ImageResource::read() {
    stbi_set_flip_vertically_on_load(true);

    buffer = stbi_load(m_Info.file.path.c_str(), &m_Resolution.x, &m_Resolution.y, &m_Channels, 0);

    if (!buffer) {
        Logger::WARNING(std::format("[ResourceHandle<Image>] Could not read image from {}: {}", m_Info.file.path, stbi_failure_reason()));
        return;
    }
}

void ImageResource::write() {
    if (!buffer) {
        Logger::WARNING("[ResourceHandle<Image>] Buffer is empty, cannot write image.");
        return;
    }

    bool ret = stbi_write_png(m_Info.file.path.c_str(), m_Resolution.x, m_Resolution.y, m_Channels, buffer, m_Resolution.x * m_Channels);

    if (!ret) {
        Logger::WARNING(std::format("[ResourceHandle<Image>] Failed to write image to {}", m_Info.file.path));
        return;
    }
}

const glm::ivec2 &ImageResource::getResolution() const {
    return m_Resolution;
}

const int &ImageResource::getChannels() const {
    return m_Channels;
}

} // namespace Physbuzz
