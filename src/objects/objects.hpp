#pragma once

#include "../scene/scene.hpp"
#include <glm/glm.hpp>

struct AABBComponent {
    glm::vec3 min;
    glm::vec3 max;
};

struct RadiusComponent {
    float radius;
};

// An AABB box.
Object &create_box(Scene &scene, glm::vec3 position, float width, float height);

// A basic circle.
Object &create_circle(Scene &scene, glm::vec3 position, float radius);
