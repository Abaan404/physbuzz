#include "texture.hpp"

#include "../debug/logging.hpp"

namespace Physbuzz {

Texture2DResource::Texture2DResource(const Texture2DInfo &texture2D)
    : m_Info(texture2D) {}

Texture2DResource::~Texture2DResource() {}

bool Texture2DResource::build() {
    std::vector<bool> &claimedUnits = getClaimedUnits();

    for (; m_Unit < claimedUnits.size(); m_Unit++) {
        if (!claimedUnits[m_Unit]) {
            claimedUnits[m_Unit] = true;
            break;
        }
    }

    if (m_Unit >= claimedUnits.size()) {
        Logger::ERROR("[Texture2DResource] TextureArray is full, cannot allocate.");
        return false;
    }

    if (m_Info.image.file.path.empty()) {
        return false;
    }

    GLint maxUnits;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
    if (m_Unit > maxUnits) {
        Logger::ERROR("[Texture2DResource] TextureUnit is out of range, supported ranges: {}", maxUnits);
        return false;
    }

    // OpenGL's origin for textures are on its top-left
    m_Info.image.flipVertically = true;
    ImageResource image = ImageResource(m_Info.image);
    if (!image.build()) {
        Logger::ERROR("[Texture2D] Could not build image: {}", m_Info.image.file.path.string());
        return false;
    }

    if (!image.read()) {
        Logger::ERROR("[Texture2D] Could not load image: {}", m_Info.image.file.path.string());
        image.destroy();
        return false;
    }

    glGenTextures(1, &m_Texture);
    glActiveTexture(GL_TEXTURE0 + m_Unit);
    glBindTexture(GL_TEXTURE_2D, m_Texture);

    // Texture Wrapping (Repeat)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint format;
    switch (image.getChannels()) {
    case 1:
        format = GL_RED;
        break;
    case 2:
        format = GL_RG;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        UNREACHABLE("[Texture2D] Too many channels.");
        break;
    }

    const glm::ivec2 &resolution = image.getResolution();
    glTexImage2D(GL_TEXTURE_2D, 0, format, resolution.x, resolution.y, 0, format, GL_UNSIGNED_BYTE, image.buffer);
    glGenerateMipmap(GL_TEXTURE_2D);

    image.destroy();
    return true;
}

bool Texture2DResource::destroy() {
    glDeleteTextures(1, &m_Texture);
    getClaimedUnits()[m_Unit] = false;

    return true;
}

bool Texture2DResource::bind() const {
    glActiveTexture(GL_TEXTURE0 + m_Unit);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    return true;
}

bool Texture2DResource::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

const GLint &Texture2DResource::getUnit() const {
    return m_Unit;
}

} // namespace Physbuzz
