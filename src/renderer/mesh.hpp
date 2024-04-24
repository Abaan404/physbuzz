#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

class MeshComponent {
  public:
    MeshComponent();
    MeshComponent(const MeshComponent &other);
    MeshComponent &operator=(const MeshComponent &other);
    ~MeshComponent();

    void set_program(GLuint program);
    void set_vertex(std::vector<glm::vec3> &vertex_buffer);
    void set_index(std::vector<glm::uvec3> &index_buffer);

    GLuint get_program();
    std::vector<glm::vec3> get_vertex();
    std::vector<glm::uvec3> get_index();

    bool normalized = false;

    // TODO uniforms class abstraction
    int glu_time;
    int glu_timedelta;
    int glu_resolution;

    bool preserve_buffers = true;

  private:
    void copy(const MeshComponent &other);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int VBO, VAO, EBO;

    GLuint program;

    friend class Renderer;
};
