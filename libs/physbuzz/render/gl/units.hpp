#pragma once

#include <cstddef>
#include <glad/gl.h>
#include <unistd.h>

namespace Physbuzz {

namespace GL {

namespace TextureUnits {

void reset();
bool deactivate(GLint unit);

GLint activate();
GLint activate(GLint unit);

std::size_t size();

}; // namespace TextureUnits

} // namespace GL

} // namespace Physbuzz
