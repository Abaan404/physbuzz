#include "image.hpp"

#include "logging.hpp"
#include <ImfArray.h>
#include <ImfRgba.h>
#include <ImfRgbaFile.h>
#include <cstring>
#include <half.h>
#include <span>

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
        const std::filesystem::path &path = m_Info.files[i].path;
        std::size_t offset = i * m_Data.meta.size;

        bool success = true;

        if (path.extension() == ".exr") {
            success &= readExr(path, offset);
        } else {
            // fallback to stb
            success &= readStb(path, offset);
        }

        if (!success) {
            m_Data = {};
            return false;
        }
    }

    return true;
}

bool ImageFile::write(const Info &info, const Data &data) {
    if (data.image.size() != data.meta.size * info.files.size()) {
        Logger::ERROR("[ImageFile] Image meta.size or properties are invalid");
        return false;
    }

    for (std::size_t i = 0; i < info.files.size(); i++) {
        const std::filesystem::path &path = info.files[i].path;
        std::size_t offset = i * data.meta.size;

        bool success = true;

        if (path.extension() == ".exr") {
            success &= writeExr(path, offset, data);
        } else {
            // fallback to stb
            success &= writeStb(path, offset, data);
        }

        if (!success) {
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
        const std::filesystem::path &path = m_Info.files[i].path;

        std::optional<Data::Meta> meta = {};

        if (path.extension() == ".exr") {
            meta = readExrMeta(path);
        } else {
            // fallback to stb
            meta = readStbMeta(path);
        }

        if (!meta) {
            m_Data.meta = {};
            return false;
        }

        if (m_Data.meta.resolution == glm::uvec2{0, 0}) {
            m_Data.meta = *meta;
        }

        if (m_Data.meta.hdr != meta->hdr) {
            Logger::ERROR("[ImageFile] Unequal HDR/LDR image from {}", path.string());
            m_Data.meta = {};
            return false;
        }

        if (m_Data.meta.resolution != meta->resolution) {
            Logger::ERROR("[ImageFile] Unequal image resolution from {}", path.string());
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

// https://cary-ilm-openexr.readthedocs.io/en/latest/ReadingAndWritingImageFiles.html
bool ImageFile::readExr(const std::filesystem::path &path, std::size_t offset) {
    try {
        Imf::RgbaInputFile file(path.c_str());
        const Imath::Box2i &dw = file.dataWindow();
        int x = dw.max.x - dw.min.x + 1;
        int y = dw.max.y - dw.min.y + 1;

        std::vector<Imf::Rgba> pixels(x * y);

        if (m_Info.flipVertically) {
            Imf::Rgba *base = Imf::ComputeBasePointer(pixels.data() + (y - 1) * x, dw.min, x, 1, -x);
            file.setFrameBuffer(base, 1, -x);
        } else {
            Imf::Rgba *base = Imf::ComputeBasePointer(pixels.data(), dw);
            file.setFrameBuffer(base, 1, x);
        }

        file.readPixels(dw.min.y, dw.max.y);

        std::span<const std::byte> bytes = std::as_bytes(std::span(pixels));

        if (offset + bytes.size() > m_Data.image.size()) {
            Logger::ERROR("[ImageFile] Offset out of bounds reading image from {}", path.string());
            return false;
        }

        std::copy(bytes.begin(), bytes.end(), m_Data.image.begin() + offset);

    } catch (const std::exception &error) {
        Logger::ERROR("[ImageFile] Could not read image from {}: {}", path.string(), error.what());
        return false;
    }

    return true;
}

bool ImageFile::readStb(const std::filesystem::path &path, std::size_t offset) {
    int x, y;
    std::size_t count;

    imath_half_bits_t *dst = reinterpret_cast<imath_half_bits_t *>(m_Data.image.data() + offset);

    if (m_Data.meta.hdr) {
        float *buffer = stbi_loadf(path.c_str(), &x, &y, nullptr, STBI_rgb_alpha);

        if (!buffer) {
            Logger::ERROR("[ImageFile] Could not read image from {}: {}", path.string(), stbi_failure_reason());
            return false;
        }

        count = x * y * STBI_rgb_alpha;

        for (std::size_t pixel = 0; pixel < count; pixel++) {
            dst[pixel] = imath_float_to_half(buffer[pixel]);
        }

        stbi_image_free(buffer);
    } else {
        stbi_uc *buffer = stbi_load(path.c_str(), &x, &y, nullptr, STBI_rgb_alpha);

        if (!buffer) {
            Logger::ERROR("[ImageFile] Could not read image from {}: {}", path.string(), stbi_failure_reason());
            return false;
        }

        count = x * y * STBI_rgb_alpha;

        for (std::size_t pixel = 0; pixel < count; pixel++) {
            constexpr float maxPixelDepth = 255.0f;
            dst[pixel] = imath_float_to_half(buffer[pixel] / maxPixelDepth);
        }

        stbi_image_free(buffer);
    }

    return true;
}

// https://cary-ilm-openexr.readthedocs.io/en/latest/ReadingAndWritingImageFiles.html
bool ImageFile::writeExr(const std::filesystem::path &path, std::size_t offset, const Data &data) {
    try {
        int x = data.meta.resolution.x;
        int y = data.meta.resolution.y;
        Imf::RgbaOutputFile file(path.c_str(), x, y, Imf::WRITE_RGBA);

        if (offset + data.meta.size > data.image.size()) {
            Logger::ERROR("[ImageFile] Offset out of bounds writing image to {}", path.string());
            return false;
        }

        const auto *pixels = reinterpret_cast<const Imf::Rgba *>(data.image.data() + offset);

        if (m_Info.flipVertically) {
            const Imf::Rgba *base = Imf::ComputeBasePointer(pixels + (y - 1) * x, file.dataWindow().min, x, 1, -x);
            file.setFrameBuffer(base, 1, -x);
        } else {
            const Imf::Rgba *base = Imf::ComputeBasePointer(pixels, file.dataWindow());
            file.setFrameBuffer(base, 1, x);
        }

        file.writePixels(y);

    } catch (const std::exception &error) {
        Logger::ERROR("[ImageFile] Failed to write image to {}: {}", path.string(), error.what());
        return false;
    }

    return true;
}

bool ImageFile::writeStb(const std::filesystem::path &path, std::size_t offset, const Data &data) {
    const std::byte *buffer = data.image.data() + offset;

    bool success = stbi_write_png(path.c_str(), data.meta.resolution.x, data.meta.resolution.y, STBI_rgb_alpha, buffer, data.meta.resolution.x * STBI_rgb_alpha);

    if (!success) {
        Logger::ERROR("[ImageFile] Failed to write image to {}", path.string());
        return false;
    }

    return true;
}

std::optional<ImageFile::Data::Meta> ImageFile::readExrMeta(const std::filesystem::path &path) const {
    try {
        Imf::RgbaInputFile file = {path.c_str()};
        const Imath::Box2i &dw = file.dataWindow();

        int x = dw.max.x - dw.min.x + 1;
        int y = dw.max.y - dw.min.y + 1;

        return Data::Meta{
            .resolution = {x, y},
            .size = x * y * sizeof(Imf::Rgba),
            .hdr = true,
        };

    } catch (const std::exception &error) {
        Logger::ERROR("[ImageFile] Could not read image meta from {}: {}", path.string(), error.what());
        return std::nullopt;
    }
}

std::optional<ImageFile::Data::Meta> ImageFile::readStbMeta(const std::filesystem::path &path) const {
    int x, y;

    if (!stbi_info(path.c_str(), &x, &y, nullptr)) {
        Logger::ERROR("[ImageFile] Could not read image meta from {}: {}", path.string(), stbi_failure_reason());
        return std::nullopt;
    }

    return Data::Meta{
        .resolution = {x, y},
        .size = static_cast<std::size_t>(x * y * STBI_rgb_alpha * sizeof(imath_half_bits_t)),
        .hdr = static_cast<bool>(stbi_is_hdr(path.c_str())),
    };
}

} // namespace Physbuzz
