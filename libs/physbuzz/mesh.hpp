#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

class MeshComponent {
  public:
    MeshComponent();
    MeshComponent(const MeshComponent &other);
    MeshComponent &operator=(const MeshComponent &other);
    ~MeshComponent();

    void build();
    void destroy();
    void redraw();

    GLuint getProgram() const;
    std::vector<glm::vec3> getVertex() const;
    std::vector<glm::uvec3> getIndex() const;

    void setProgram(GLuint program);
    void setVertex(std::vector<glm::vec3> &vertexBuffer);
    void setIndex(std::vector<glm::uvec3> &indexBuffer);

    // TODO uniforms class abstraction
    int gluTime;
    int gluTimedelta;
    int gluResolution;

  private:
    void copy(const MeshComponent &other);

    std::vector<float> m_ScreenVertices;
    std::vector<float> m_Vertices;
    std::vector<unsigned int> m_Indices;
    unsigned int VBO, VAO, EBO;

    GLuint m_Program;

    friend class Renderer;
};

} // namespace Physbuzz
