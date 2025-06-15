#pragma once

#include <glad/gl.h>

namespace Physbuzz {

namespace GL {

enum class DepthFunc {
    Never = GL_NEVER,
    Less = GL_LESS,
    Equal = GL_EQUAL,
    LEqual = GL_LEQUAL,
    Greater = GL_GREATER,
    NotEqual = GL_NOTEQUAL,
    GEqual = GL_GEQUAL,
    Always = GL_ALWAYS,
};

inline static void setDepthMask(const bool mask) {
    glDepthMask(mask ? GL_TRUE : GL_FALSE);
}

inline static GLboolean getDepthMask() {
    GLboolean depthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    return depthMask;
}

inline static void setDepthFunc(const DepthFunc func = DepthFunc::Less) {
    glDepthFunc(static_cast<GLenum>(func));
}

inline static DepthFunc getDepthFunc() {
    GLint depthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    return static_cast<DepthFunc>(depthFunc);
}

} // namespace GL

} // namespace Physbuzz
