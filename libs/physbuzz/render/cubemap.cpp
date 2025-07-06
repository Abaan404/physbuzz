#include "cubemap.hpp"

#include "../debug/logging.hpp"
#include "gl/units.hpp"

namespace Physbuzz {

Cubemap::Cubemap(const Info &info)
    : m_Info(info) {}

Cubemap::~Cubemap() {}

bool Cubemap::build() {
    if (m_Texture != 0) {
        Logger::WARNING("[Cubemap] Trying to build a built cubemap.");
        return true;
    }

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Texture);

    glTextureParameteri(m_Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_Texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_Texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_Texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    std::array images = {
        ImageFile(m_Info.right),
        ImageFile(m_Info.left),
        ImageFile(m_Info.top),
        ImageFile(m_Info.bottom),
        ImageFile(m_Info.front),
        ImageFile(m_Info.back),
    };

    glm::ivec2 resolution;
    int channels = 0;
    bool consistent = true;

    for (size_t i = 0; i < images.size(); i++) {
        if (!images[i].build() || !images[i].read()) {
            Logger::ERROR("[Cubemap] Could not load image for face {}", i);
            consistent = false;
            continue;
        }

        const auto &data = images[i].getData();
        if (i == 0) {
            resolution = data.resolution;
            channels = data.channels;
        } else if (data.resolution != resolution || data.channels != channels) {
            Logger::ERROR("[Cubemap] Face {} has different resolution/channels", i);
            consistent = false;
        }
    }

    if (!consistent) {
        for (auto &image : images) {
            image.destroy();
        }
        destroy();
        return false;
    }

    GLenum internalFormat, format;
    switch (channels) {
    case 1:
        internalFormat = GL_R8;
        format = GL_RED;
        break;
    case 2:
        internalFormat = GL_RG8;
        format = GL_RG;
        break;
    case 3:
        internalFormat = GL_RGB8;
        format = GL_RGB;
        break;
    case 4:
        internalFormat = GL_RGBA8;
        format = GL_RGBA;
        break;
    default:
        UNREACHABLE("[Cubemap] Invalid channel count");
        destroy();
        return false;
    }

    glTextureStorage2D(m_Texture, 1, internalFormat, resolution.x, resolution.y);

    for (size_t i = 0; i < images.size(); i++) {
        glTextureSubImage3D(m_Texture, 0, 0, 0, i, resolution.x, resolution.y, 1, format, GL_UNSIGNED_BYTE, images[i].getData().image.data());
        images[i].destroy();
    }

    return true;
}

bool Cubemap::destroy() {
    if (m_Texture == 0) {
        Logger::WARNING("[Cubemap] Trying to destroy a destructed cubemap.");
        return true;
    }

    glDeleteTextures(1, &m_Texture);

    return true;
}

GLint Cubemap::activate(GLint unit) const {
    PBZ_ASSERT(m_Texture != 0, "[Cubemap] trying to activate an incomplete cubemap.");
    return GL::detail::TextureUnits::activate(m_Texture, unit);
}

const Cubemap::Info &Cubemap::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
