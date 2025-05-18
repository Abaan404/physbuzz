#include "cubemap.hpp"

#include "../debug/logging.hpp"
#include "texture.hpp"

namespace Physbuzz {

CubemapResource::CubemapResource(const CubemapInfo &info)
    : m_Info(info) {}

CubemapResource::~CubemapResource() {}

bool CubemapResource::build() {
    std::vector<bool> &claimedUnits = getClaimedUnits();

    for (; m_Unit < claimedUnits.size(); m_Unit++) {
        if (!claimedUnits[m_Unit]) {
            claimedUnits[m_Unit] = true;
            break;
        }
    }

    if (m_Unit >= claimedUnits.size()) {
        Logger::ERROR("[CubemapResource] TextureArray is full, cannot allocate.");
        return false;
    }

    glGenTextures(1, &m_Texture);
    glActiveTexture(GL_TEXTURE0 + m_Unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture);

    bool success = true;

    success &= loadImage(m_Info.right, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    success &= loadImage(m_Info.left, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    success &= loadImage(m_Info.top, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    success &= loadImage(m_Info.bottom, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    success &= loadImage(m_Info.front, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
    success &= loadImage(m_Info.back, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

    if (!success) {
        destroy();
        return false;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return true;
}

bool CubemapResource::destroy() {
    glDeleteTextures(1, &m_Texture);
    getClaimedUnits()[m_Unit] = false;

    return true;
}

bool CubemapResource::bind(bool disableDepth) const {
    if (disableDepth) {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
    }

    glActiveTexture(GL_TEXTURE0 + m_Unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture);
    return true;
}

bool CubemapResource::unbind(bool enableDepth) const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    if (enableDepth) {
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
    return true;
}

const GLint &CubemapResource::getUnit() const {
    return m_Unit;
}

bool CubemapResource::loadImage(ImageInfo &imageInfo, GLenum target) {
    if (imageInfo.file.path.empty()) {
        return false;
    }

    ImageResource image = ImageResource(imageInfo);
    if (!image.build()) {
        Logger::ERROR("[CubemapResource] Could not build image: {}", imageInfo.file.path.string());
        return false;
    }

    if (!image.read()) {
        Logger::ERROR("[CubemapResource] Could not load image: {}", imageInfo.file.path.string());
        image.destroy();
        return false;
    }

    const glm::vec2 &resolution = image.getResolution();
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
        UNREACHABLE("[CubemapResource] Too many channels.");
        break;
    }

    glTexImage2D(target, 0, format, resolution.x, resolution.y, 0, format, GL_UNSIGNED_BYTE, image.buffer);

    image.destroy();
    return true;
}

} // namespace Physbuzz
