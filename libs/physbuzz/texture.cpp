#include "texture.hpp"

#include "glad/gl.h"
#include "logging.hpp"
#include <algorithm>
#include <format>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Physbuzz {

Texture::Texture(const GLint &unit)
    : m_Unit(unit) {
    static GLint limit = maxUnits();
    Logger::ASSERT(unit < limit, std::format("TextureUnit is out of range, supported ranges: {}", limit));
}

Texture::~Texture() {}

void Texture::build(const std::string &path) {
    glGenTextures(1, &m_Texture);
    glActiveTexture(GL_TEXTURE0 + m_Unit);
    glBindTexture(GL_TEXTURE_2D, m_Texture);

    // Texture Wrapping (Repeat)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        Logger::ERROR(std::format("Could not generate Texture2D \"{}\": {}", path, stbi_failure_reason()));
    }

    glActiveTexture(GL_TEXTURE0 + m_Unit);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

void Texture::destroy() {
    glDeleteTextures(1, &m_Texture);
}

void Texture::bind() const {
    glActiveTexture(GL_TEXTURE0 + m_Unit);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

const GLuint &Texture::getTexture() const {
    return m_Texture;
}

const GLint &Texture::getUnit() const {
    return m_Unit;
}

GLint Texture::maxUnits() {
    GLint maxUnits;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
    return maxUnits;
}

TextureArray::TextureArray() {}

TextureArray::~TextureArray() {}

std::unordered_map<std::string, Texture> TextureArray::m_Map;
std::vector<bool> TextureArray::m_ClaimedUnits;

void TextureArray::build() {
    m_ClaimedUnits.resize(Texture::maxUnits(), false);
}

void TextureArray::destroy() {
    for (auto &[_, texture] : m_Map) {
        texture.destroy();
    }

    m_Map.clear();
    std::fill(m_ClaimedUnits.begin(), m_ClaimedUnits.end(), false);
}

Texture &TextureArray::allocate(const std::string &identifier, const std::string &path) {
    GLint unit = fetchEmptyUnit();
    Logger::ASSERT(unit != -1, std::format("TextureArray is full, cannot allocate \"{}\"...", identifier));

    Texture texture = Texture(unit);
    texture.build(path);
    m_Map[identifier] = std::move(texture);
    m_ClaimedUnits[unit] = true;

    return m_Map[identifier];
}

void TextureArray::deallocate(const std::string &identifier) {
    if (!m_Map.contains(identifier)) {
        Logger::WARNING(std::format("Texture \"{}\" not found.", identifier));
        return;
    }

    Texture &texture = m_Map[identifier];
    texture.destroy();
    m_Map.erase(identifier);
    m_ClaimedUnits[texture.getUnit()] = false;
}

Texture &TextureArray::getTexture(const std::string &identifier) {
    if (!m_Map.contains(identifier)) {
        Logger::WARNING(std::format("Texture \"{}\" not found.", identifier));
    }

    return m_Map[identifier];
}

GLint TextureArray::fetchEmptyUnit() {
    for (std::size_t i = 0; i < m_ClaimedUnits.size(); i++) {
        if (!m_ClaimedUnits[i]) {
            return i;
        }
    }

    return -1;
}

} // namespace Physbuzz
