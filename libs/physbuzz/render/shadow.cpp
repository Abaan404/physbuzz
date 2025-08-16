#include "shadow.hpp"

#include "../ecs/scene.hpp"
#include "lighting.hpp"
#include "renderer.hpp"
#include <array>
#include <format>

namespace Physbuzz {

namespace Builtin {

bool ShaderShadowDepth2D::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    // return ResourceRegistry<ShaderPipeline>::insert(
    //     Resource.getIdentifier(),
    //     {{
    //         .draw = [](const ShaderPipeline *resource, Scene &scene, ObjectID object) {
    //             const auto [render] = scene.getComponent<RenderComponent>(object);
    //
    //             resource->setUniform("PBZ_Model", render.transform.matrix);
    //
    //             for (const auto &[mesh, _] : render.model->getMeshs()) {
    //                 mesh.draw();
    //             }
    //         },
    //     }});

    return true;
}

bool ShaderShaderDepthCubemap::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    // return ResourceRegistry<ShaderPipeline>::insert(
    //     Resource.getIdentifier(),
    //     {{
    //         .draw = [](const ShaderPipeline *resource, Scene &scene, ObjectID object) {
    //             const auto [render] = scene.getComponent<RenderComponent>(object);
    //
    //             resource->setUniform("PBZ_Model", render.transform.matrix);
    //
    //             for (const auto &[mesh, _] : render.model->getMeshs()) {
    //                 mesh.draw();
    //             }
    //         },
    //     }});

    return true;
}

} // namespace Builtin

Shadow::Shadow(const Info &info, const glm::ivec2 &resolution)
    : m_Info(info),
      m_Framebuffers({
          .directional = {{
              .resolution = resolution,
              .colors = {},
              .depth = {
                  .storage = Framebuffer::Storage::Texture2D,
              },
          }},
          .point = {{
              .resolution = glm::ivec2(glm::max(resolution.x, resolution.y)),
              .colors = {},
              .depth = {
                  .storage = Framebuffer::Storage::Cubemap,
              },
          }},
      }) {}

bool Shadow::build() {
    bool success = true;

    success &= Builtin::ShaderRendererPassthrough::build();
    success &= Builtin::ShaderShadowDepth2D::build();
    success &= Builtin::ShaderShaderDepthCubemap::build();

    success &= m_Framebuffers.directional.build();
    success &= m_Framebuffers.point.build();

    return success;
}
bool Shadow::destroy() {
    bool success = true;

    success &= m_Framebuffers.point.destroy();
    success &= m_Framebuffers.directional.destroy();

    return success;
}

void Shadow::tick() const {
    // GLint cullMode;
    // glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);
    //
    // glCullFace(GL_FRONT);
    //
    // tickDirectional();
    // tickPoint();
    //
    // glCullFace(static_cast<GLenum>(cullMode));
}

void Shadow::tickDirectional() const {
    // m_Framebuffers.directional.clear();
    // m_Framebuffers.directional.bind();
    //
    // glm::mat4 projection = glm::ortho(-m_Info.orthoSize, m_Info.orthoSize, -m_Info.orthoSize, m_Info.orthoSize, 1.0f, m_Info.depth);
    //
    // if (!Builtin::ShaderShadowDepth2D::Resource->reload()) {
    //     return;
    // }
    //
    // Builtin::ShaderShadowDepth2D::Resource->bind();
    // for (const auto [_, light] : m_Scene->getComponents<DirectionalLightComponent>()) {
    //     glm::mat4 view = glm::lookAt(-light.direction * m_Info.depth / 2.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    //     light.matrix = projection * view;
    //
    //     Builtin::ShaderShadowDepth2D::Resource->setUniform("PBZ_ShadowMatrix", light.matrix);
    //
    //     for (const auto &object : m_Objects) {
    //         Builtin::ShaderShadowDepth2D::Resource->draw(*m_Scene, object);
    //     }
    // }
    //
    // Builtin::ShaderShadowDepth2D::Resource->unbind();
    // m_Framebuffers.directional.unbind();
}

void Shadow::tickPoint() const {
    // m_Framebuffers.point.clear();
    // m_Framebuffers.point.bind();
    //
    // glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, m_Info.depth);
    //
    // if (!Builtin::ShaderShaderDepthCubemap::Resource->reload()) {
    //     return;
    // }
    //
    // Builtin::ShaderShaderDepthCubemap::Resource->bind();
    // Builtin::ShaderShaderDepthCubemap::Resource->setUniform("PBZ_FarPlane", m_Info.depth);
    //
    // for (const auto [_, light] : m_Scene->getComponents<PointLightComponent>()) {
    //     std::array matrices = {
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(1.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(-1.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 1.0f, 0.0f), {0.0f, 0.0f, 1.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, -1.0f, 0.0f), {0.0f, 0.0f, -1.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 0.0f, 1.0f), {0.0f, -1.0f, 0.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 0.0f, -1.0f), {0.0f, -1.0f, 0.0f}),
    //     };
    //
    //     Builtin::ShaderShaderDepthCubemap::Resource->setUniform("PBZ_LightPosition", light.position);
    //     for (std::size_t i = 0; i < matrices.size(); i++) {
    //         Builtin::ShaderShaderDepthCubemap::Resource->setUniform(std::format("PBZ_LightMatrix[{}]", i), matrices[i]);
    //     }
    //
    //     for (const auto &object : m_Objects) {
    //         Builtin::ShaderShaderDepthCubemap::Resource->draw(*m_Scene, object);
    //     }
    // }
    //
    // Builtin::ShaderShaderDepthCubemap::Resource->unbind();
    // m_Framebuffers.point.unbind();
}

void Shadow::resize(const glm::ivec2 &resolution) {
    m_Framebuffers.directional.resize(resolution);
    m_Framebuffers.point.resize(glm::ivec2(glm::max(resolution.x, resolution.y)));
}

const Shadow::Framebuffers &Shadow::getFramebuffers() const {
    return m_Framebuffers;
}

const Shadow::Info &Shadow::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
