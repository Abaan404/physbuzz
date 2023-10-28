#pragma once

#include "geometry/box.hpp"
#include "physics/physics_box.hpp"
#include <type_traits>
#include <vector>

// TODO encapsulate vectors to disallow direct modification of vectors (if im arsed to)
struct GameObjectStore {
    std::vector<Box> boxes;
    std::vector<PhysicsBox> phys_boxes;

    std::vector<GameObject> objects;

    // just adds objects to objects and deducing type to the respective vector
    template<typename T, typename = std::is_base_of<T, GameObject>>
    void push_object(T object) {
        if constexpr (std::derived_from<T, Box>)
            boxes.push_back(object);

        if constexpr (std::derived_from<T, PhysicsBox>)
            phys_boxes.push_back(object);

        objects.push_back(object);
    }
};
