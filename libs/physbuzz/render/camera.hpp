#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Physbuzz {

struct CameraInfo {
    enum class Projection {
        Perspective,
        Orthographic,
        Unknown,
    } projection = Projection::Unknown;

    struct Orthographic {
        float left = 0.0f;
        float right = 1.0f;
        float bottom = 1.0f;
        float top = 0.0f;
    } orthographic;

    struct Perspective {
        float fovy = glm::radians(45.0f);
        float aspect = 1.0f;
    } perspective;

    struct Depth {
        float near = -1.0f;
        float far = 1.0f;
    } depth;

    struct View {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    } view;

    glm::ivec2 resolution = {1, 1};
};

class CameraComponent {
  public:
    CameraComponent(const CameraInfo &info);

    void resize(const glm::ivec2 &resolution);
    void reset();

    void setProjection(CameraInfo::Projection projection);
    void update(const CameraInfo &info);

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
    const CameraInfo &getInfo() const;

  private:
    void updateProjection();
    void updateView();

    glm::mat4 m_Projection{1.0f};
    glm::mat4 m_View{1.0f};

    CameraInfo m_Info;
};

} // namespace Physbuzz
