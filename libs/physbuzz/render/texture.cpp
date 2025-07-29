#include "texture.hpp"

#include "../debug/logging.hpp"
#include "gl/units.hpp"

namespace Physbuzz {

Texture2D::Texture2D(const Info &texture2D)
    : m_Info(texture2D) {}

Texture2D::~Texture2D() {}

bool Texture2D::build() {
    if (m_Texture != 0) {
        Logger::WARNING("[Texture2D] Trying to build a built texture.");
        return true;
    }

    if (m_Info.image.file.path.empty()) {
        return false;
    }

    // OpenGL's origin for textures are on its top-left
    m_Info.image.flipVertically = true;
    ImageFile image = ImageFile(m_Info.image);
    if (!image.build()) {
        Logger::ERROR("[Texture2D] Could not build image: {}", m_Info.image.file.path.string());
        return false;
    }

    if (!image.read()) {
        Logger::ERROR("[Texture2D] Could not load image: {}", m_Info.image.file.path.string());
        image.destroy();
        return false;
    }

    const ImageFile::Data &data = image.getData();

    glCreateTextures(GL_TEXTURE_2D, 1, &m_Texture);

    glTextureParameteri(m_Texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_Texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format;
    GLenum internalFormat;
    switch (data.channels) {
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
        UNREACHABLE("[Texture2D] Too many channels.");
        break;
    }

    glTextureStorage2D(m_Texture, 1, internalFormat, data.resolution.x, data.resolution.y);
    glTextureSubImage2D(m_Texture, 0, 0, 0, data.resolution.x, data.resolution.y, format, GL_UNSIGNED_BYTE, data.image.data());
    glGenerateTextureMipmap(m_Texture);

    image.destroy();
    return true;
}

bool Texture2D::destroy() {
    if (m_Texture == 0) {
        Logger::WARNING("[Texture2D] Trying to destroy a destructed texture.");
        return true;
    }

    glDeleteTextures(1, &m_Texture);
    return true;
}

const Texture2D::Info &Texture2D::getInfo() const {
    return m_Info;
}

GLint Texture2D::activate(GLint unit) const {
    PBZ_ASSERT(m_Texture != 0, "[Texture2D] trying to activate an incomplete texture.");
    return GL::detail::TextureUnits::activate(m_Texture, unit);
}

GLuint Texture2D::getImGuiTextureHandle() const {
    PBZ_ASSERT(m_Texture != 0, "[Texture2D] trying to create an imgui handle on an incomplete texture.");
    return m_Texture;
}

} // namespace Physbuzz
