#include "shadow.hpp"

#include "../ecs/scene.hpp"
#include "../resources/builtins/meshes.hpp"
#include "../resources/builtins/shaders.hpp"
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

    glm::mat4 projection = glm::ortho(-m_Info.orthoSize, m_Info.orthoSize, -m_Info.orthoSize, m_Info.orthoSize, 1.0f, m_Info.depth);
    Builtin::Depth::Resource->bind();

    if (!Builtin::Depth::Resource->reload()) {
        return;
    }

    GLint cullMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);

    glCullFace(GL_FRONT);

    for (const auto [directional] : scene.getComponents<DirectionalLightComponent>()) {
        glm::mat4 view = glm::lookAt(-directional.direction * m_Info.depth / 2.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
        directional.matrix = projection * view;

        Builtin::Depth::Resource->setUniform("u_LightMatrix", directional.matrix);

        for (const auto &object : m_Objects) {
            Builtin::Depth::Resource->draw(scene, object);
        }
    }

    glCullFace(static_cast<GLenum>(cullMode));

    Builtin::Depth::Resource->unbind();
}

void Shadow::resize(const glm::ivec2 &resolution) {
    m_Framebuffer.resize(resolution);
}

} // namespace Physbuzz
