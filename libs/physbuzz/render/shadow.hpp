#pragma once

#include "../ecs/system.hpp"
#include "framebuffer.hpp"
#include "renderers/defines.hpp"
#include "shaders.hpp"

namespace Physbuzz {

namespace Builtin {

namespace ShaderShadowDepth2D {

inline Resource<RenderPipeline> Resource = {"builtin/depth/2D"};

bool build();

} // namespace ShaderShadowDepth2D

namespace ShaderShaderDepthCubemap {

inline Resource<RenderPipeline> Resource = {"builtin/depth/cubemap"};

bool build();

} // namespace ShaderShaderDepthCubemap

} // namespace Builtin

struct ShadowComponent {};

class Shadow : public System<RenderComponent, ShadowComponent> {
  public:
    struct Framebuffers {
        Framebuffer directional;
        Framebuffer point;
    };

    struct Info {
        float orthoSize = 100.0f;
        float depth = 100.0f;
    };

    Shadow(const Info &info, const glm::ivec2 &resolution);

    bool build();
    bool destroy();

    void resize(const glm::ivec2 &resolution);

    void tick() const;

    const Framebuffers &getFramebuffers() const;
    const Info &getInfo() const;

  private:
    void tickDirectional() const;
    void tickPoint() const;

    Info m_Info;
    Framebuffers m_Framebuffers;
};

} // namespace Physbuzz
