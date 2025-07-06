#include "image.hpp"

#include "../debug/logging.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace Physbuzz {

ImageFile::ImageFile(const Info &image)
    : m_Info(image) {}

ImageFile::~ImageFile() {}

bool ImageFile::build() {
    return true;
}

bool ImageFile::destroy() {
    return true;
}

bool ImageFile::read() {
    stbi_set_flip_vertically_on_load(m_Info.flipVertically);

    stbi_uc *buffer = stbi_load(m_Info.file.path.c_str(), &m_Data.resolution.x, &m_Data.resolution.y, &m_Data.channels, 0);

    if (!buffer) {
        Logger::ERROR("[ImageFile] Could not read image from {}: {}", m_Info.file.path.string(), stbi_failure_reason());
        return false;
    }

    m_Data.image = {buffer, buffer + (m_Data.resolution.x * m_Data.resolution.y * m_Data.channels)};
    stbi_image_free(buffer);

    return true;
}

bool ImageFile::write(const Info &info, const Data &data) {
    if (data.resolution.x * data.resolution.y * data.channels != data.image.size()) {
        Logger::ERROR("[ImageFile] Image size or properties are invalid");
        return false;
    }

    bool ret = stbi_write_png(info.file.path.c_str(), data.resolution.x, data.resolution.y, data.channels, data.image.data(), data.resolution.x * data.channels);

    if (!ret) {
        Logger::ERROR("[ImageFile] Failed to write image to {}", info.file.path.string());
        return false;
    }

    m_Info = info;
    m_Data = data;

    return true;
}

const ImageFile::Data &ImageFile::getData() const {
    return m_Data;
}

const ImageFile::Info &ImageFile::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
