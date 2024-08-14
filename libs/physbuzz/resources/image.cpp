#include "image.hpp"

#include "physbuzz/debug/logging.hpp"
#include <format>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace Physbuzz {

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
