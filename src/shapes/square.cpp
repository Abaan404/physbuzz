#include "square.hpp"

Physbuzz::Geometry<Physbuzz::Model::Vertex> ModelSquare::build() {
    constexpr float min = -0.5f;
    constexpr float max = 0.5f;

    return {
        .vertices = {
            {{min, min, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // top-left
            {{min, max, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // top-right
            {{max, max, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // bottom-right
            {{max, min, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // bottom-left
        },
        .indices = {0, 1, 2, 2, 3, 0},
    };
}
