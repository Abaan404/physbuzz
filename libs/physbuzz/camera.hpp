#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Physbuzz {

class Camera {
  public:
    Camera();
    ~Camera();

    void build();
    void destroy();

    enum class ProjectionType {
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

    void updateView();
    void resetView();
    void resize(const glm::ivec2 &resolution, const Depth &depth);

    // specialization for 2D projection
    void setOrthographic(const glm::ivec2 &resolution);
    void setOrthographic(const Orthographic &orthographic, const Depth &depth);
    const Orthographic &getOrthographic() const;

    void setPrespective(const Prespective &prespective, const Depth &depth);
    const Prespective &getPrespective() const;

    const Depth &getDepth() const;
    void setDepth(const Depth &depth);

    void translate(const glm::vec3 &delta);
    void rotate(const glm::quat &delta);

    void setView(const glm::mat4 &view);
    const glm::mat4 &getView() const;

    void setProjection(const glm::mat4 &projection);
    const glm::mat4 &getProjection() const;

    const ProjectionType &getProjectionType() const;

    void setPosition(const glm::vec3 &position);
    const glm::vec3 &getPosition() const;

    void setOrientation(const glm::quat &orientaton);
    const glm::quat &getOrientation() const;

    void setFacing(const glm::vec3 &facing);
    const glm::vec3 getFacing() const;

    void setUp(const glm::vec3 &up);
    const glm::vec3 getUp() const;

    void setRight(const glm::vec3 &right);
    const glm::vec3 getRight() const;

  private:
    glm::mat4 m_View = glm::mat4(1.0f);
    glm::mat4 m_Projection = glm::mat4(1.0f);

    glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
    glm::quat m_Orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    Orthographic m_Orthographic;
    Prespective m_Prespective;
    Depth m_Depth;

    ProjectionType m_ProjectionType = ProjectionType::Unknown;
};

} // namespace Physbuzz
