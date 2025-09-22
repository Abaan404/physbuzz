#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Physbuzz {

class CameraComponent {
  public:
    enum class Projection {
        Perspective,
        Orthographic,
        Unknown,
    };

    struct Orthographic {
        float left = 0.0f;
        float right = 1.0f;
        float bottom = 1.0f;
        float top = 0.0f;
    };

    struct Perspective {
        float fovy = glm::radians(45.0f);
        float aspect = 1.0f;
    };

    struct Depth {
        float near = -1.0f;
        float far = 1.0f;
    };

    struct View {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    };

    struct Info {
        Projection projection = Projection::Unknown;
        Orthographic orthographic;
        Perspective perspective;
        Depth depth;
        View view;

        glm::ivec2 resolution = {1, 1};
    };

    CameraComponent(const Info &info);

    void resize(const glm::ivec2 &resolution);
    void reset();

    void setProjection(CameraComponent::Projection projection);
    void update(const Info &info);

    void setPosition(const glm::vec3 &position);
    void setOrientation(const glm::quat &orientation);

    void setFacing(const glm::vec3 &facing);
    void setUp(const glm::vec3 &up);
    void setRight(const glm::vec3 &right);

    glm::vec3 getFacing() const;
    glm::vec3 getUp() const;
    glm::vec3 getRight() const;

    const glm::mat4 &getProjection() const;
    const glm::mat4 &getView() const;
    const Info &getInfo() const;

  private:
    void updateProjection();
    void updateView();

    glm::mat4 m_Projection = {1.0f};
    glm::mat4 m_View = {1.0f};

    Info m_Info;
};

} // namespace Physbuzz
