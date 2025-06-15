#pragma once

#include "../io/image.hpp"
#include <glad/gl.h>
#include <vector>

namespace Physbuzz {

enum class TextureType;

struct Texture2DInfo {
    ImageInfo image;
    TextureType type;
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
    TextureType getType() const;

  private:
    Texture2DInfo m_Info;
    GLuint m_Texture;
    GLint m_Unit = 0;
};

/** A virtual mirror of claimed units in the GPU as described in the OpenGL spec. Temporary
 *  implementation detail until the engine moves to Vulkan
 *  Note: could investigate https://registry.khronos.org/OpenGL/extensions/ARB/ARB_bindless_texture.txt */
std::vector<bool> &getClaimedUnits();

} // namespace Physbuzz
