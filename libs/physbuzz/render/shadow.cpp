#include "shadow.hpp"

#include "../ecs/scene.hpp"
#include "../resources/builtins/meshes.hpp"
#include "../resources/builtins/shaders.hpp"
#include "gl/capabilities.hpp"
#include "gl/units.hpp"
#include "lighting.hpp"

namespace Physbuzz {

Shadow::Shadow(const ShadowInfo &info)
    : m_Info(info),
      m_Framebuffer({
          .resolution = info.resolution,
          .colorClear = {0.0f, 0.0f, 0.0f, 0.0f},
          .colors = {},
          .depth = {
              .storage = DepthAttachmentInfo::Storage::Texture,
          },
          .output = {
              .type = OutputAttachmentInfo::Type::Depth,
          },
      }) {}

bool Shadow::build() {
    bool success = true;

    success &= Builtin::ScreenQuad::build();
    success &= Builtin::Passthrough::build();
    success &= Builtin::Depth::build();

    success &= m_Framebuffer.build();

    return success;
}
bool Shadow::destroy() {
    return m_Framebuffer.destroy();
}

const Framebuffer &Shadow::getFramebuffer() {
    return m_Framebuffer;
}

void Shadow::tick(Scene &scene) const {
    m_Framebuffer.clear();
    m_Framebuffer.bind();

    glm::mat4 lightProjection = glm::ortho(-m_Info.orthoSize, m_Info.orthoSize, -m_Info.orthoSize, m_Info.orthoSize, 1.0f, m_Info.depth);
    Builtin::Depth::Resource->bind();

    if (!Builtin::Depth::Resource->reload()) {
        return;
    }

    for (const auto [directional] : scene.getComponents<DirectionalLightComponent>()) {
        glm::mat4 lightView = glm::lookAt(directional.direction * m_Info.depth / 2.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
        glm::mat4 lightMatrix = lightProjection * lightView;

        Builtin::Depth::Resource->setUniform("u_LightMatrix", lightMatrix);

        for (const auto &object : m_Objects) {
            Builtin::Depth::Resource->draw(scene, object);
        }
    }

    Builtin::Depth::Resource->unbind();

    GL::TextureUnits::reset();
    m_Framebuffer.bindOutputTexture(0);
    m_Framebuffer.unbind();

    bool depthTest = GL::getCapability(GL::Capabilities::DepthTest);
    GL::setCapability(GL::Capabilities::DepthTest, false);

    Builtin::Passthrough::Resource->bind();
    Builtin::Passthrough::Resource->setUniform("PBZ_Framebuffer", 0);
    for (const auto &[mesh, _] : Builtin::ScreenQuad::Resource->getMeshs()) {
        mesh.draw();
    }
    Builtin::Passthrough::Resource->unbind();

    m_Framebuffer.unbindOutputTexture();
    GL::setCapability(GL::Capabilities::DepthTest, depthTest);
}

} // namespace Physbuzz
