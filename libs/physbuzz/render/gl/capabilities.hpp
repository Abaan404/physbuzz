#pragma once

#include <glad/gl.h>

namespace Physbuzz {

namespace GL {

enum class Capabilities {
    Blend = GL_BLEND,
    CullFace = GL_CULL_FACE,
    DebugOutput = GL_DEBUG_OUTPUT,
    DebugOutputSynchronous = GL_DEBUG_OUTPUT_SYNCHRONOUS,
    DepthTest = GL_DEPTH_TEST,
    ScissorTest = GL_SCISSOR_TEST,
    StencilTest = GL_STENCIL_TEST,
};

inline static void setCapability(const Capabilities capability, const bool enable) {
    if (enable) {
        glEnable(static_cast<GLenum>(capability));
    } else {
        glDisable(static_cast<GLenum>(capability));
    }
}

inline static bool getCapability(const Capabilities capability) {
    return glIsEnabled(static_cast<GLenum>(capability)) == GL_TRUE;
}

} // namespace GL

} // namespace Physbuzz
