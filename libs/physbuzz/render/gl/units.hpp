#pragma once

#include <cstddef>
#include <glad/gl.h>
#include <unistd.h>

namespace Physbuzz {

namespace GL {

namespace detail {

namespace TextureUnits {

void reset();
bool deactivate(GLint unit);
GLint activate(GLuint texture, GLint unit = -1);

std::size_t size();

} // namespace TextureUnits

}; // namespace detail

} // namespace GL

} // namespace Physbuzz
