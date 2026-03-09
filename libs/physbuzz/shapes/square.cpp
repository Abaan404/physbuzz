#include "square.hpp"

#include "../graphics/mesh.hpp"
#include "../graphics/model.hpp"

namespace Physbuzz {

namespace Builtin {

bool ModelSquare::build(TransferBatch &batch) {
    if (ResourceRegistry<Mesh>::contains(Resource)) {
        return true;
    }

    Physbuzz::Mesh::Info info = {
        .description = &Model::Vertex::Description,
        .vertexCount = 4,
        .indexCount = 6,
    };

    if (!Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(Resource, info)) {
        return false;
    }

    constexpr float min = -0.5f;
    constexpr float max = 0.5f;

    return Resource->write<Model::Vertex>(
        {
            {{min, min, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // top-left
            {{min, max, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // top-right
            {{max, max, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // bottom-right
            {{max, min, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // bottom-left
        },
        {0, 1, 2, 2, 3, 0},
        batch);
}

} // namespace Builtin

} // namespace Physbuzz
