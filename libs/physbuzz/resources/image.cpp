#include "image.hpp"

#include "../debug/logging.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace Physbuzz {

ImageResource::ImageResource(const ImageInfo &image)
    : m_Info(image) {}

ImageResource::~ImageResource() {}

bool ImageResource::build() {
    return true;
}

bool ImageResource::destroy() {
    if (buffer != nullptr) {
        stbi_image_free(buffer);
        buffer = nullptr;
    }

    return true;
}

bool ImageResource::read() {
    stbi_set_flip_vertically_on_load(m_Info.flipVertically);

    buffer = stbi_load(m_Info.file.path.c_str(), &m_Resolution.x, &m_Resolution.y, &m_Channels, 0);

    if (!buffer) {
        Logger::ERROR("[ImageResource] Could not read image from {}: {}", m_Info.file.path.string(), stbi_failure_reason());
        return false;
    }

    return true;
}

bool ImageResource::write() {
    if (!buffer) {
        Logger::ERROR("[ImageResource] Buffer is empty, cannot write image.");
        return false;
    }

    bool ret = stbi_write_png(m_Info.file.path.c_str(), m_Resolution.x, m_Resolution.y, m_Channels, buffer, m_Resolution.x * m_Channels);

    if (!ret) {
        Logger::ERROR("[ImageResource] Failed to write image to {}", m_Info.file.path.string());
        return false;
    }

    return true;
}

const glm::ivec2 &ImageResource::getResolution() const {
    return m_Resolution;
}

const int &ImageResource::getChannels() const {
    return m_Channels;
}

} // namespace Physbuzz
