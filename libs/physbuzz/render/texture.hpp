#pragma once

#include "../resources/image.hpp"
#include "../resources/manager.hpp"
#include <glad/gl.h>

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

    // to access m_Unit in container
    template <ResourceType T>
    friend class ResourceContainer;
};

template <>
bool ResourceContainer<Texture2DResource>::insert(const std::string &identifier, Texture2DResource &&resource);

template <>
bool ResourceContainer<Texture2DResource>::erase(const std::string &identifier);

namespace {

/** A virtual mirror of claimed units in the GPU as described in the OpenGL spec. Temporary
 *  implementation detail until the engine moves to Vulkan */
static std::vector<bool> &getClaimedUnits() {
    static std::vector<bool> claimedUnits;

    static int runOnce = []() {
        GLint maxUnits;
        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
        claimedUnits.resize(maxUnits, false);

        return 0;
    }();

    return claimedUnits;
}

} // namespace

} // namespace Physbuzz
