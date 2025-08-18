#pragma once

#include "../../ecs/system.hpp"
#include "../../resources/resources.hpp"
#include "../framebuffer.hpp"
#include "../shaders.hpp"
#include "defines.hpp"

namespace Physbuzz {

namespace Builtin {

namespace ShaderDeferredGeometry {

inline Resource<ShaderPipeline> Resource = {"builtin/deferred/geometry"};

bool build();

} // namespace ShaderDeferredGeometry

namespace ShaderDeferredLighting {

inline Resource<ShaderPipeline> Resource = {"builtin/deferred/lighting"};

bool build();

} // namespace ShaderDeferredLighting

} // namespace Builtin

struct RenderComponent;

struct DeferredRenderComponent {
    struct ForwardPass {
        Resource<ShaderPipeline> pipeline;
    };
};

class DeferredRenderer : public IRenderer,
                         public System<RenderComponent, DeferredRenderComponent> {
  public:
    struct Framebuffers {
        Framebuffer gBuffer;
        Framebuffer output;
    };

    struct Info {
        std::uint32_t gBufferCount = 3;

        struct {
            Resource<ShaderPipeline> geometry = Builtin::ShaderDeferredGeometry::Resource;
            Resource<ShaderPipeline> lighting = Builtin::ShaderDeferredLighting::Resource;
        } passes = {};
    };

    DeferredRenderer(const Info &info, const glm::ivec2 &resolution);

    bool build() override;
    bool destroy() override;

    void resize(const glm::ivec2 &resolution) override;

    void tick(const vk::CommandBuffer &commandBuffer) const;
    void render(ObjectID id) const;

    const Framebuffers &getFramebuffers() const;
    const Framebuffer &getOutput() const override;
    const Info &getInfo() const;

  private:
    Info m_Info;

    Framebuffers m_Framebuffers;
};

} // namespace Physbuzz
