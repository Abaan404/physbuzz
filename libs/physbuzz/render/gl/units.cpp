#include "units.hpp"

#include "../../debug/logging.hpp"
#include <glad/gl.h>
#include <mutex>
#include <vector>

namespace Physbuzz {

namespace GL {

namespace detail {

inline static std::once_flag onceFlag;
inline static std::vector<bool> units;

void setup() {
    std::call_once(onceFlag, [] {
        GLint maxUnits;
        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
        units.resize(maxUnits, false);
    });
}

void TextureUnits::reset() {
    setup();

    std::fill(units.begin(), units.end(), false);
}

bool TextureUnits::deactivate(GLint unit) {
    setup();

    if (unit >= units.size()) {
        return false;
    }

    units[unit] = false;
    return true;
}

GLint TextureUnits::activate(GLuint texture, GLint unit) {
    setup();

    if (unit < 0) {
        for (unit = 0; unit < units.size(); unit++) {
            if (!units[unit]) {
                break;
            }
        }
    }

    if (unit >= units.size() || units[unit]) {
        Logger::ERROR("[GL] Could not activate a texture because no units are available or the unit was already claimed.");
        return -1;
    }

    units[unit] = true;
    glBindTextureUnit(unit, texture);
    return unit;
}

std::size_t TextureUnits::size() {
    setup();
    return units.size();
}

} // namespace detail

} // namespace GL

} // namespace Physbuzz
