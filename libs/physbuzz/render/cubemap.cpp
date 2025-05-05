#include "cubemap.hpp"

#include "glad/gl.h"
#include "texture.hpp"

namespace Physbuzz {

CubemapResource::CubemapResource(const CubemapInfo &info)
    : m_Info(info) {}

CubemapResource::~CubemapResource() {}

bool CubemapResource::build() {
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
    return true;
}

bool CubemapResource::bind(bool depthMask) const {
    if (depthMask) {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
    }

    glActiveTexture(GL_TEXTURE0 + m_Unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture);
    return true;
}

bool CubemapResource::unbind(bool depthMask) const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    if (depthMask) {
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
    }
    return true;
}

const GLint &CubemapResource::getUnit() const {
    return m_Unit;
}

std::uint8_t CubemapResource::loadImage(ImageInfo &imageInfo, GLenum target) {
    if (imageInfo.file.path.empty()) {
        return false;
    }

    GLint maxUnits;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
    if (m_Unit > maxUnits) {
        Logger::ERROR("[CubemapResource] TextureUnit is out of range, supported ranges: {}", maxUnits);
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

template <>
bool ResourceContainer<CubemapResource>::insert(const std::string &identifier, CubemapResource &&resource) {
    std::vector<bool> &claimedUnits = getClaimedUnits();

    GLint unit = -1;
    for (std::size_t i = 0; i < claimedUnits.size(); i++) {
        if (!claimedUnits[i]) {
            claimedUnits[i] = true;
            unit = i;
            break;
        }
    }

    if (unit == -1) {
        Logger::ERROR("[CubemapResource] TextureArray is full, cannot allocate \"{}\".", identifier);
        return false;
    }

    resource.m_Unit = unit;
    return ResourceContainer::base_insert(identifier, std::move(resource));
}

template <>
bool ResourceContainer<CubemapResource>::erase(const std::string &identifier) {
    if (!m_Map.contains(identifier)) {
        Logger::WARNING("[CubemapResource] Tried to erase non-existent texture \"{}\".", identifier);
        return false;
    }

    CubemapResource &texture = m_Map.get(identifier);
    getClaimedUnits()[texture.m_Unit] = false;

    return base_erase(identifier);
}

} // namespace Physbuzz
