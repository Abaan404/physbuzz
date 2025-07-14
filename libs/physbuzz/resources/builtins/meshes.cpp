#include "meshes.hpp"

#include "vertices.hpp"

namespace Physbuzz {

namespace Builtin {

bool MeshScreenQuad::build() {
    if (ResourceRegistry<Model>::contains(Resource.getIdentifier())) {
        return true;
    }

    if (!VertexScreenQuad::build()) {
        return false;
    }

    return ResourceRegistry<Model>::insert(
        Resource.getIdentifier(),
        {{
            .meshes = {
                {
                    {
                        Mesh::Info<VertexScreenQuad::Format>{
                            .attribute = {VertexScreenQuad::Resource.getIdentifier()},
                            .vertices = {
                                {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
                                {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
                                {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
                                {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                            },
                            .indices = {0, 1, 2, 2, 3, 0},
                        },
                        {},
                    },
                },
            },
        }});
}

} // namespace Builtin

} // namespace Physbuzz
