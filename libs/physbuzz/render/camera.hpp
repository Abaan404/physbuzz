#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Physbuzz {

struct CameraInfo {
    enum class Type {
        Prespective,
        Orthographic,
        Orthographic2D,
        Unknown,
    } type;

    struct Orthographic {
        float left = 0.0f;
        float right = 1.0f;
        float bottom = 1.0f;
        float top = 0.0f;
    } orthographic;

    struct Prespective {
        float fovy = glm::radians(45.0f);
        float aspect = 1.0f;
    } prespective;

    struct Depth {
        float near = -1.0f;
        float far = 1.0f;
    } depth;

    struct View {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    } view;

    glm::ivec2 resolution;
};

class CameraComponent {
  public:
    CameraComponent(const CameraInfo &info);

    void resize(const glm::ivec2 &resolution);

    void setOrthographic2D(const glm::ivec2 &resolution);
    void setOrthographic(const CameraInfo::Orthographic &orthographic);
    void setPrespective(const CameraInfo::Prespective &prespective);
    void setDepth(const CameraInfo::Depth &depth);

    const glm::mat4 &getProjection() const;

    void reset();

    void setFacing(const glm::vec3 &facing);
    const glm::vec3 getFacing() const;

    void setUp(const glm::vec3 &up);
    const glm::vec3 getUp() const;

    void setRight(const glm::vec3 &right);
    const glm::vec3 getRight() const;

    void setPosition(const glm::vec3 &position);
    void setOrientation(const glm::quat &orientation);

    const glm::mat4 &getView() const;

    const CameraInfo &getInfo() const;

  private:
    void update();

    glm::mat4 m_Projection = glm::mat4(1.0f);
    glm::mat4 m_View = glm::mat4(1.0f);

    CameraInfo m_Info;
};

} // namespace Physbuzz
