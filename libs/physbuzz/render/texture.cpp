#include "texture.hpp"

namespace Physbuzz {

Texture2DResource::Texture2DResource(const Texture2DInfo &texture2D)
    : m_Info(texture2D) {}

Texture2DResource::~Texture2DResource() {}

void Texture2DResource::build() {
    if (m_Info.image.file.path.empty()) {
        return;
    }

    GLint maxUnits;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
    if (m_Unit > maxUnits) {
        Logger::ERROR("[Texture2DResource] TextureUnit is out of range, supported ranges: {}", maxUnits);
    }

    ImageResource image = ImageResource(m_Info.image);
    image.build();
    image.read();

    if (image.buffer == nullptr) {
        Logger::ERROR("[Texture2D] Could not load image: {}", m_Info.image.file.path);
        image.destroy();
        return;
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

    const glm::ivec2 &resolution = image.getResolution();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, image.buffer);
    glGenerateMipmap(GL_TEXTURE_2D);

    image.destroy();
}

void Texture2DResource::destroy() {
    glDeleteTextures(1, &m_Texture);
}

void Texture2DResource::bind() const {
    glActiveTexture(GL_TEXTURE0 + m_Unit);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
}

void Texture2DResource::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

const GLint &Texture2DResource::getUnit() const {
    return m_Unit;
}

/** A virtual mirror of claimed units in the GPU as described in the OpenGL spec. Temporary
 *  implementation detail until the engine moves to Vulkan */
static std::vector<bool> &getClaimedUnits() {
    static std::vector<bool> claimedUnits;

    static int runOnce = []() {
        GLint maxUnits;
        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
        claimedUnits.resize(maxUnits, false);

        return 0;
    }();

    return claimedUnits;
}

template <>
void ResourceContainer<Texture2DResource>::destroy() {
    std::vector<bool> &claimedUnits = getClaimedUnits();
    std::fill(claimedUnits.begin(), claimedUnits.end(), false);

    ResourceContainer::base_destroy();
}

template <>
void ResourceContainer<Texture2DResource>::insert(const std::string &identifier, const Texture2DResource &resource) {
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
        Logger::ERROR("[Texture2DResource] TextureArray is full, cannot allocate \"{}\"...", identifier);
    }

    Texture2DResource newResource = resource;
    newResource.m_Unit = unit;

    ResourceContainer::base_insert(identifier, newResource);
}

template <>
bool ResourceContainer<Texture2DResource>::remove(const std::string &identifier) {
    Texture2DResource &texture = m_Map.get(identifier);
    getClaimedUnits()[texture.m_Unit] = false;

    return base_remove(identifier);
}

} // namespace Physbuzz
