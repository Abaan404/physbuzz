#pragma once

#include "../../ecs/system.hpp"
#include "../../resources/resources.hpp"
#include "../framebuffer.hpp"
#include "../shaders.hpp"
#include "defines.hpp"

namespace Physbuzz {

namespace Builtin {

namespace ShaderDeferredGeometry {

inline Resource<RenderPipeline> Resource = {"builtin/deferred/geometry"};

bool build();

} // namespace ShaderDeferredGeometry

namespace ShaderDeferredLighting {

inline Resource<RenderPipeline> Resource = {"builtin/deferred/lighting"};

bool build();

} // namespace ShaderDeferredLighting

} // namespace Builtin

struct RenderComponent;

struct DeferredRenderComponent {
    struct ForwardPass {
        Resource<RenderPipeline> pipeline;
    };
};

class DeferredRenderer : public IRenderPass,
                         public System<RenderComponent, DeferredRenderComponent> {
  public:
    struct Framebuffers {
        Framebuffer gBuffer;
        Framebuffer output;
    };

    struct Info {
        std::uint32_t gBufferCount = 3;

        struct {
            Resource<RenderPipeline> geometry = Builtin::ShaderDeferredGeometry::Resource;
            Resource<RenderPipeline> lighting = Builtin::ShaderDeferredLighting::Resource;
        } passes = {};
    };

    DeferredRenderer(const Info &info, const glm::ivec2 &resolution);

    bool build() override;
    bool destroy() override;

    void render(const vk::CommandBuffer &commandBuffer, std::uint32_t frameInFlight) override;

    const Framebuffers &getFramebuffers() const;
    const Framebuffer &getOutput() const;
    const Info &getInfo() const;

  private:
    Info m_Info;

    Framebuffers m_Framebuffers;
};

} // namespace Physbuzz
