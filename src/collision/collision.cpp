#include "collision.hpp"

bool Collision::collides(std::shared_ptr<GameObject> object1, std::shared_ptr<GameObject> object2) {
    switch (object1->identifier) {
    case Objects::PhysicsBox:
        switch (object2->identifier) {
        case Objects::PhysicsBox:
            return check_collision_box_box(*std::static_pointer_cast<PhysicsBox>(object1), *std::static_pointer_cast<PhysicsBox>(object2));

        case Objects::PhysicsCircle:
            return check_collision_box_circle(*std::static_pointer_cast<PhysicsBox>(object1), *std::static_pointer_cast<PhysicsCircle>(object2));

        default:
            break;
        }
        break;
    case Objects::PhysicsCircle:
        switch (object2->identifier) {
        case Objects::PhysicsCircle:
            return check_collision_circle_circle(*std::static_pointer_cast<PhysicsCircle>(object1), *std::static_pointer_cast<PhysicsCircle>(object2));

        case Objects::PhysicsBox:
            return check_collision_box_circle(*std::static_pointer_cast<PhysicsBox>(object2), *std::static_pointer_cast<PhysicsCircle>(object1));

        default:
            break;
        }
        break;
    default:
        break;
    }

    return false;
}

// box rotation not implemented yet
bool Collision::check_collision_box_box(PhysicsBox &box1, PhysicsBox &box2) {
    if (box1.max.x < box2.min.x or box1.min.x > box2.max.x)
        return false;

    if (box1.max.y < box2.min.y or box1.min.y > box2.max.y)
        return false;

    printf("BoxBox Collision\n");
    return true;
}

bool Collision::check_collision_circle_circle(PhysicsCircle &circle1, PhysicsCircle &circle2) {
    const float dist_squared = (circle2.position.x - circle1.position.x) * (circle2.position.x - circle1.position.x) +
                               (circle2.position.y - circle1.position.y) * (circle2.position.y - circle1.position.y);

    const float r_squared = (circle1.radius + circle2.radius) * (circle1.radius + circle2.radius);

    if (dist_squared > r_squared)
        return false;

    printf("CircleCircle Collision\n");
    return true;
}

// box rotation not implemented yet
bool Collision::check_collision_box_circle(PhysicsBox &box, PhysicsCircle &circle) {
    glm::vec2 point;
    point.x = glm::clamp(circle.position.x, box.min.x, box.max.x);
    point.y = glm::clamp(circle.position.y, box.min.y, box.max.y);

    glm::vec2 dist = circle.position - point;
    float dist_squared = glm::dot(dist, dist);
    float r_squared = circle.radius * circle.radius;

    if (dist_squared > r_squared)
        return false;

    printf("Circle-Box Collision\n");
    return true;
}
