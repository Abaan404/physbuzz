#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

class Texture;
class Attachment;
class DynamicBuffer;
struct RenderContext;

struct OnTextureRebuild {
    Texture *texture;
    const RenderContext &context;
};

struct OnAttachmentRebuild {
    Attachment *attachment;
    const RenderContext &context;
};

struct OnDynamicBufferRebuild {
    DynamicBuffer *buffer;
    const RenderContext &context;
};

} // namespace Physbuzz
