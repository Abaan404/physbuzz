#include "shadow.hpp"

#include "../ecs/scene.hpp"
#include "../resources/builtins/meshes.hpp"
#include "../resources/builtins/shaders.hpp"
#include "lighting.hpp"
#include <array>
#include <format>

namespace Physbuzz {

Shadow::Shadow(const Info &info)
    : m_Info(info),
      m_Framebuffers({
          .directional = {{
              .resolution = info.resolution,
              .colors = {},
              .depth = {
                  .storage = Framebuffer::Storage::Texture2D,
              },
              .output = {
                  .type = Framebuffer::Type::Depth,
              },
          }},
          .point = {{
              .resolution = glm::ivec2(glm::max(info.resolution.x, info.resolution.y)),
              .colors = {},
              .depth = {
                  .storage = Framebuffer::Storage::Cubemap,
              },
              .output = {
                  .type = Framebuffer::Type::Depth,
              },
          }},
      }) {}

bool Shadow::build() {
    bool success = true;

    success &= Builtin::MeshScreenQuad::build();
    success &= Builtin::ShaderPassthrough::build();
    success &= Builtin::ShaderDepth2D::build();
    success &= Builtin::ShaderDepthCubemap::build();

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

void Shadow::tick(Scene &scene) const {
    GLint cullMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);

    glCullFace(GL_FRONT);

    tickDirectional(scene);
    tickPoint(scene);

    glCullFace(static_cast<GLenum>(cullMode));
}

void Shadow::tickDirectional(Scene &scene) const {
    m_Framebuffers.directional.clear();
    m_Framebuffers.directional.bind();

    glm::mat4 projection = glm::ortho(-m_Info.orthoSize, m_Info.orthoSize, -m_Info.orthoSize, m_Info.orthoSize, 1.0f, m_Info.depth);

    if (!Builtin::ShaderDepth2D::Resource->reload()) {
        return;
    }

    Builtin::ShaderDepth2D::Resource->bind();
    for (const auto [light] : scene.getComponents<DirectionalLightComponent>()) {
        glm::mat4 view = glm::lookAt(-light.direction * m_Info.depth / 2.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
        light.matrix = projection * view;

        Builtin::ShaderDepth2D::Resource->setUniform("PBZ_ShadowMatrix", light.matrix);

        for (const auto &object : m_Objects) {
            Builtin::ShaderDepth2D::Resource->draw(scene, object);
        }
    }

    Builtin::ShaderDepth2D::Resource->unbind();
    m_Framebuffers.directional.unbind();
}

void Shadow::tickPoint(Scene &scene) const {
    m_Framebuffers.point.clear();
    m_Framebuffers.point.bind();

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, m_Info.depth);

    if (!Builtin::ShaderDepthCubemap::Resource->reload()) {
        return;
    }

    Builtin::ShaderDepthCubemap::Resource->bind();
    Builtin::ShaderDepthCubemap::Resource->setUniform("PBZ_FarPlane", m_Info.depth);

    for (const auto [light] : scene.getComponents<PointLightComponent>()) {
        std::array matrices = {
            projection * glm::lookAt(light.position, light.position + glm::vec3(1.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}),
            projection * glm::lookAt(light.position, light.position + glm::vec3(-1.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}),
            projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 1.0f, 0.0f), {0.0f, 0.0f, 1.0f}),
            projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, -1.0f, 0.0f), {0.0f, 0.0f, -1.0f}),
            projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 0.0f, 1.0f), {0.0f, -1.0f, 0.0f}),
            projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 0.0f, -1.0f), {0.0f, -1.0f, 0.0f}),
        };

        Builtin::ShaderDepthCubemap::Resource->setUniform("PBZ_LightPosition", light.position);
        for (std::size_t i = 0; i < matrices.size(); i++) {
            Builtin::ShaderDepthCubemap::Resource->setUniform(std::format("PBZ_LightMatrix[{}]", i), matrices[i]);
        }

        for (const auto &object : m_Objects) {
            Builtin::ShaderDepthCubemap::Resource->draw(scene, object);
        }
    }

    Builtin::ShaderDepthCubemap::Resource->unbind();
    m_Framebuffers.point.unbind();
}

void Shadow::resize(const glm::ivec2 &resolution) {
    m_Info.resolution = resolution;
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
