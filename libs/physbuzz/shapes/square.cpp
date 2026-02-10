#include "square.hpp"

#include "../graphics/mesh.hpp"
#include "../graphics/model.hpp"

namespace Physbuzz {

namespace Builtin {

bool ModelSquare::build(const std::shared_ptr<Transfer> transfer) {
    if (ResourceRegistry<Mesh>::contains(Resource)) {
        return true;
    }

    constexpr float min = -0.5f;
    constexpr float max = 0.5f;

    Physbuzz::Mesh::Info<Physbuzz::Model::Vertex> mesh = {
        .vertices = {
            {{min, min, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // top-left
            {{min, max, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // top-right
            {{max, max, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // bottom-right
            {{max, min, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // bottom-left
        },
        .indices = {0, 1, 2, 2, 3, 0},
    };

    return Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(Resource, mesh, transfer);
}

} // namespace Builtin

} // namespace Physbuzz
