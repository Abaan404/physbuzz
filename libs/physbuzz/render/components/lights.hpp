#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

class DirectionalLightComponent {
  public:
    struct Info {
        glm::vec3 direction = {0.0f, 0.0f, 0.0f};
        glm::vec3 intensity = {0.0f, 0.0f, 0.0f};

        float orthoSize = 10.0f;
        float depth = 10.0f;
    };

    DirectionalLightComponent(const Info &info);

    void setDirection(const glm::vec3 &direction);
    void setIntensity(const glm::vec3 &intensity);

    void update(const Info &info);

    const Info &getInfo() const;
    const glm::mat4 &getProjectionView() const;

  private:
    void updateProjectionView();

    Info m_Info;

    glm::mat4 m_ProjectionView = {1.0f};
};

struct PointLightComponent {
  public:
    struct Info {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 intensity = {0.0f, 0.0f, 0.0f};

        float depth = 10.0f;
    };

    PointLightComponent(const Info &info);

    void setPosition(const glm::vec3 &position);
    void setIntensity(const glm::vec3 &intensity);

    void update(const Info &info);

    const Info &getInfo() const;
    const std::array<glm::mat4, 6> &getProjectionView() const;

  private:
    void updateProjectionView();

    Info m_Info;

    std::array<glm::mat4, 6> m_ProjectionView = {1.0f};
};

struct SpotLightComponent {
  public:
    struct Info {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 direction = {0.0f, 0.0f, 0.0f};
        glm::vec3 intensity = {0.0f, 0.0f, 0.0f};

        float cutOff = glm::cos(glm::radians(12.5f));
        float outerCutOff = glm::cos(glm::radians(17.5f));
    };

    SpotLightComponent(const Info &info);

    void setPosition(const glm::vec3 &position);
    void setDirection(const glm::vec3 &direction);
    void setIntensity(const glm::vec3 &intensity);
    void setCutoff(float inner, float outer);

    void update(const Info &info);

    const Info &getInfo() const;

  private:
    Info m_Info;

    glm::mat4 m_ProjectionView = {1.0f};
};

} // namespace Physbuzz
