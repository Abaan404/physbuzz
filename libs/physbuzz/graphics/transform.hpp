#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Physbuzz {

class Transform {
  public:
    struct Info {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};
        glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    };

    Transform(const Info &info);

    const glm::vec3 toWorld(const glm::vec3 &local) const;
    const glm::vec3 toLocal(const glm::vec3 &world) const;

    void setPosition(const glm::vec3 position);
    void setScale(const glm::vec3 scale);
    void setOrientation(const glm::quat orientation);
    void setOrientation(float magnitude, const glm::vec3 &direction);

    void update(const Info &info);

    const Info &getInfo() const;
    const glm::mat4 &getModel() const;

  private:
    void updateModel();

    Info m_Info;

    glm::mat4 m_Model = glm::mat4(1.0f);
};

} // namespace Physbuzz
