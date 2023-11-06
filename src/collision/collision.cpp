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
    if (box1.max[0] < box2.min[0] or box1.min[0] > box2.max[0])
        return false;

    if (box1.max[1] < box2.min[1] or box1.min[1] > box2.max[1])
        return false;

    printf("BoxBox Collision\n");
    return true;
}

bool Collision::check_collision_circle_circle(PhysicsCircle &circle1, PhysicsCircle &circle2) {
    const float dist_squared = (circle2.position[0] - circle1.position[0]) * (circle2.position[0] - circle1.position[0])
        + (circle2.position[1] - circle1.position[1]) * (circle2.position[1] - circle1.position[1]);
    const float r_squared = (circle1.radius + circle2.radius) * (circle1.radius + circle2.radius);

    if ((dist_squared - r_squared) > 0)
        return false;

    printf("CircleCircle Collision\n");
    return true;
}

// box rotation not implemented yet
bool Collision::check_collision_box_circle(PhysicsBox &box, PhysicsCircle &circle) {
    return false;
    // int x_vert, y_vert;
    //
    // // check which quadrant the box is in hence which vertex to use
    // if (box.x > circle.x and box.y > circle.y) {
    //     x_vert = box.max[0];
    //     y_vert = box.min[1];
    //
    // } else if (box.x < circle.x and box.y > circle.y) {
    //     x_vert = box.max[0];
    //     y_vert = box.max[1];
    //
    // } else if (box.x > circle.x and box.y < circle.y) {
    //     x_vert = box.min[0];
    //     y_vert = box.min[1];
    //
    // } else if (box.x < circle.x and box.y < circle.y) {
    //     x_vert = box.min[0];
    //     y_vert = box.max[1];
    // } else {
    //     throw std::runtime_error("Could not find the vertex for BoxCircle collision");
    // }
    //
    // float theta = std::tan((y_vert - circle.y) / (x_vert - circle.x));
    // float x_hit = circle.radius * std::cos(theta);
    // float y_hit = circle.radius * std::sin(theta);
    //
    // float diagonal_half = std::sqrt((box.width / 2) * (box.width / 2) + (box.height / 2) * (box.height / 2));
    // float dist_squared = (x_hit - x_vert) * (x_hit - x_vert) + (y_hit - y_vert) * (y_hit - y_vert);
    //
    // if (dist_squared - ((diagonal_half + circle.radius) * (diagonal_half + circle.radius)) > 0)
    //     return false;
    //
    // printf("CircleBox Collision\n");
    // return true;
}
