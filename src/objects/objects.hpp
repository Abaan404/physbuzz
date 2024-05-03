#pragma once

#include <glm/glm.hpp>
#include <physbuzz/object.hpp>

// TODO these only exist to figure out what function to call
// for a rebuild, a rebuild callback would be better.
enum class ObjectType {
    Circle,
    Box,
};

struct IdentifiableComponent {
    ObjectType type;
    std::string_view name;
};

struct AABBComponent {
    glm::vec3 min;
    glm::vec3 max;
};

struct RadiusComponent {
    float radius;
};

// An AABB box.
void buildBox(Physbuzz::Object &box, glm::vec3 position, float width, float height);

// A basic circle.
void buildCircle(Physbuzz::Object &circle, glm::vec3 position, float radius);
