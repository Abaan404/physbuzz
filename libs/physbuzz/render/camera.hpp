#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Physbuzz {

struct View {
    void update();
    void reset();

    void rotate(const glm::quat &delta);
    void translate(const glm::vec3 &delta);

    void setFacing(const glm::vec3 &facing);
    const glm::vec3 getFacing() const;

    void setUp(const glm::vec3 &up);
    const glm::vec3 getUp() const;

    void setRight(const glm::vec3 &right);
    const glm::vec3 getRight() const;

    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 matrix = glm::mat4(1.0f);
};

class Camera {
  public:
    Camera();
    ~Camera();

    enum class Type {
        Prespective,
        Orthographic,
        Orthographic2D,
        Unknown,
    };

    struct Orthographic {
        float left = 0.0f;
        float right = 1.0f;
        float bottom = 1.0f;
        float top = 0.0f;
    };

    struct Prespective {
        float fovy = glm::radians(45.0f);
        float aspect = 1.0f;
    };

    struct Depth {
        float near = -1.0f;
        float far = 1.0f;
    };

    void resize(const glm::ivec2 &resolution, const Depth &depth);

    void setOrthographic(const glm::ivec2 &resolution);
    void setOrthographic(const Orthographic &orthographic, const Depth &depth);
    const Orthographic &getOrthographic() const;

    void setPrespective(const Prespective &prespective, const Depth &depth);
    const Prespective &getPrespective() const;

    const Depth &getDepth() const;
    void setDepth(const Depth &depth);

    void setProjection(const glm::mat4 &projection);
    const glm::mat4 &getProjection() const;

    const Type &getProjectionType() const;

    View view;

  private:
    glm::mat4 m_Projection = glm::mat4(1.0f);

    Orthographic m_Orthographic;
    Prespective m_Prespective;
    Depth m_Depth;

    Type m_ProjectionType = Type::Unknown;
};

} // namespace Physbuzz
