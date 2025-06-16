#include "units.hpp"
#include "glad/gl.h"
#include <vector>

#include <mutex>

namespace Physbuzz {

namespace GL {

inline static std::once_flag onceFlag;
inline static std::vector<bool> units;

void setup() {
    std::call_once(onceFlag, [] {
        GLint maxUnits;
        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
        units.resize(maxUnits, false);
    });
}

bool TextureUnits::deactivate(GLint unit) {
    setup();

    if (unit > units.size() - 1) {
        return false;
    }

    units[unit] = false;
    return true;
}

void TextureUnits::reset() {
    setup();

    std::fill(units.begin(), units.end(), false);
}

GLint TextureUnits::activate() {
    setup();

    std::size_t unit;
    // 0 cant be activated automatically, only manually
    for (unit = 1; unit < units.size(); unit++) {
        if (!units[unit]) {
            break;
        }
    }

    return activate(unit);
}

GLint TextureUnits::activate(GLint unit) {
    setup();

    if (unit > units.size() - 1 || units[unit]) {
        return -1;
    }

    units[unit] = true;
    glActiveTexture(GL_TEXTURE0 + unit);
    return unit;
}

std::size_t TextureUnits::size() {
    setup();
    return units.size();
}

} // namespace GL

} // namespace Physbuzz
