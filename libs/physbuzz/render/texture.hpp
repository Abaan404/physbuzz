#pragma once

#include "../io/image.hpp"
#include <glad/gl.h>
#include <mutex>
#include <vector>

namespace Physbuzz {

struct Texture2DInfo {
    ImageInfo image;
};

class Texture2DResource {
  public:
    Texture2DResource(const Texture2DInfo &texture2D);
    ~Texture2DResource();

    bool build();
    bool destroy();

    bool bind() const;
    bool unbind() const;

    const GLint &getUnit() const;

  private:
    Texture2DInfo m_Info;
    GLuint m_Texture;
    GLint m_Unit = 0;
};

namespace {

/** A virtual mirror of claimed units in the GPU as described in the OpenGL spec. Temporary
 *  implementation detail until the engine moves to Vulkan
 *  Note: could investigate https://registry.khronos.org/OpenGL/extensions/ARB/ARB_bindless_texture.txt */
static std::vector<bool> &getClaimedUnits() {
    static std::vector<bool> claimedUnits;

    static std::once_flag onceFlag;
    std::call_once(onceFlag, [] {
        GLint maxUnits;
        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
        claimedUnits.resize(maxUnits, false);
    });

    return claimedUnits;
}

} // namespace

} // namespace Physbuzz
