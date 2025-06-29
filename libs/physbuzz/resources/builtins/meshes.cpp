#include "meshes.hpp"

namespace Physbuzz {

namespace Builtin {

bool ScreenQuad::build() {
    if (ResourceRegistry<Model>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<Model>::insert(
        Resource.getIdentifier(),
        {{
            .meshes = {
                {
                    {{
                        .vertices = {
                            {{-1.0f, -1.0f, 0.0f}, {}, {0.0f, 0.0f}},
                            {{1.0f, -1.0f, 0.0f}, {}, {1.0f, 0.0f}},
                            {{1.0f, 1.0f, 0.0f}, {}, {1.0f, 1.0f}},
                            {{-1.0f, 1.0f, 0.0f}, {}, {0.0f, 1.0f}},
                        },
                        .indices = {{0, 1, 2, 2, 3, 0}},
                    }},
                    {},
                },
            },
        }});
}

} // namespace Builtin

} // namespace Physbuzz
