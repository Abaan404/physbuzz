#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace Physbuzz {

class Texture {
  public:
    Texture(const GLint &unit = 0);
    ~Texture();

    void build(const std::string &path);
    void destroy();

    void bind() const;
    void unbind() const;

    const GLuint &getTexture() const;
    const GLint &getUnit() const;

    static GLint maxUnits();

  private:
    GLuint m_Texture;
    GLint m_Unit;

    std::string m_Path;
};

// TODO rewrite me
// this isnt the commonly used "texture arrays" but rather a temp class to handle many textures until i figure out
// the actual implementation. Also multiple instances WILL cause undefined behaviours (thanks opengl)
class TextureArray {
  public:
    TextureArray();
    ~TextureArray();

    void build();
    void destroy();

    Texture &allocate(const std::string &identifier, const std::string &path);
    void deallocate(const std::string &identifier);

    Texture &getTexture(const std::string &identifier) ;

  private:
    GLint fetchEmptyUnit();

    static std::unordered_map<std::string, Texture> m_Map;
    static std::vector<bool> m_ClaimedUnits; // A virtual mirror lookup of claimed units in the GPU
};

} // namespace Physbuzz
