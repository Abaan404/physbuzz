#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Physbuzz {

using Index = glm::uvec3;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal; // (TODO implement lightmaps)
    glm::vec2 texCoords;
};

class Texture {
  public:
    Texture(const std::string &path);
    ~Texture();

    void build();
    void destroy();

    void bind();
    void unbind();

    const glm::ivec2 &getResolution() const;
    const int &getChannels() const;
    const GLuint &getId() const;

  private:
    GLuint m_Id;
    std::string m_Path;

    int m_Channels;
    glm::ivec2 m_Resolution;
};

class Mesh {
  public:
    Mesh();
    ~Mesh();

    void build();
    void destroy();

    void bind() const;
    void draw() const;
    void unbind() const;

    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    std::vector<Texture> textures;

    std::vector<glm::vec3> positions;
    bool isScaled = false;

  private:
    GLuint VBO, VAO, EBO;
};

} // namespace Physbuzz
