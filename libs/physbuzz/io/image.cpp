#include "image.hpp"

#include "logging.hpp"
#include <half.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace Physbuzz {

ImageFile::ImageFile(const Info &image)
    : m_Info(image) {}

ImageFile::~ImageFile() {}

bool ImageFile::read() {
    m_Data.image.clear();

    stbi_set_flip_vertically_on_load_thread(m_Info.flipVertically);

    if (!readMeta()) {
        return false;
    }

    m_Data.image.resize(m_Data.meta.size * m_Info.files.size());

    for (std::size_t i = 0; i < m_Info.files.size(); i++) {
        // might be worth keeping 8bit rgb and setting hdr = false in the future
        // however stbi_loadf converts LDR to HDR with stbi_ldr_to_hdr_scale(1.0f)
        // and stbi_ldr_to_hdr_gamma(2.2f) if it ever becomes an issue. but they
        // aren't thread safe.

        int x, y;
        float *buffer = stbi_loadf(m_Info.files[i].path.c_str(), &x, &y, nullptr, STBI_rgb_alpha);

        if (!buffer) {
            Logger::ERROR("[ImageFile] Could not read image from {}: {}", m_Info.files[i].path.string(), stbi_failure_reason());
            m_Data = {};
            return false;
        }

        std::size_t count = x * y * STBI_rgb_alpha;
        std::size_t offset = i * m_Data.meta.size;

        imath_half_bits_t *dst = reinterpret_cast<imath_half_bits_t *>(m_Data.image.data() + offset);
        for (std::size_t pixel = 0; pixel < count; pixel++) {
            dst[pixel] = imath_float_to_half(buffer[pixel]);
        }

        stbi_image_free(buffer);
    }

    return true;
}

bool ImageFile::write(const Info &info, const Data &data) {
    if (data.meta.resolution.x * data.meta.resolution.y * STBI_rgb_alpha != static_cast<std::int32_t>(data.image.size())) {
        Logger::ERROR("[ImageFile] Image meta.size or properties are invalid");
        return false;
    }

    for (std::size_t i = 0; i < m_Info.files.size(); i++) {
        const std::byte *buffer = data.image.data() + m_Data.image.size() * i;

        bool ret = stbi_write_png(info.files[i].path.c_str(), data.meta.resolution.x, data.meta.resolution.y, STBI_rgb_alpha, buffer, data.meta.resolution.x * STBI_rgb_alpha);

        if (!ret) {
            Logger::ERROR("[ImageFile] Failed to write image to {}", info.files[i].path.string());
            return false;
        }
    }

    m_Info = info;
    m_Data = data;

    return true;
}

bool ImageFile::readMeta() {
    m_Data.meta.resolution = {0, 0};

    for (std::size_t i = 0; i < m_Info.files.size(); i++) {
        int x, y;
        const std::filesystem::path &path = m_Info.files[i].path;

        if (!stbi_info(path.c_str(), &x, &y, nullptr)) {
            Logger::ERROR("[ImageFile] Could not read image meta from {}: {}", m_Info.files[i].path.string(), stbi_failure_reason());
            m_Data.meta = {};
            return false;
        }

        if (m_Data.meta.resolution == glm::uvec2{0, 0}) {
            m_Data.meta = {
                .resolution = {x, y},
                .size = static_cast<std::size_t>(x * y * STBI_rgb_alpha * sizeof(imath_half_bits_t)),
                .hdr = static_cast<bool>(stbi_is_hdr(path.c_str())),
            };
        }

        if (m_Data.meta.resolution != glm::uvec2{x, y}) {
            Logger::ERROR("[ImageFile] Unequal image resolution from {}", m_Info.files[i].path.string());
            m_Data.meta = {};
            return false;
        }
    }

    return true;
}

const ImageFile::Data &ImageFile::getData() const {
    return m_Data;
}

const ImageFile::Info &ImageFile::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
