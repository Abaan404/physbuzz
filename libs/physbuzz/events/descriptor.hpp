#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

class Texture;
class DynamicBuffer;
class StaticBuffer;
struct RenderContext;

struct OnTextureRealloc {
    Texture *texture;
    const RenderContext &context;
};

struct OnDynamicBufferRealloc {
    DynamicBuffer *buffer;
    const RenderContext &context;
};

struct OnStaticBufferRealloc {
    StaticBuffer *buffer;
};

} // namespace Physbuzz
