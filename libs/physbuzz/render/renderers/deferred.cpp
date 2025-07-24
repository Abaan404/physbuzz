#include "deferred.hpp"

#include "../../ecs/scene.hpp"
#include "../gl/units.hpp"
#include "../lighting.hpp"
#include "../renderer.hpp"

namespace Physbuzz {

namespace Builtin {

bool ShaderDeferredGeometry::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<ShaderPipeline>::insert(
        Resource.getIdentifier(),
        {{
            .vertex = {.file = {.path = "resources/shaders/builtin/deferred/geometry.vert"}},
            .tessControl = {},
            .tessEvaluation = {},
            .geometry = {},
            .fragment = {.file = {.path = "resources/shaders/builtin/deferred/geometry.frag"}},
            .compute = {},
            .draw = [](const ShaderPipeline *pipeline, Scene &scene, ObjectID object) {
                const auto [render] = scene.getComponent<RenderComponent>(object);

                // bind textures
                std::unordered_map<TextureType, std::uint32_t> textureLengths;

                for (const auto &texture : render.model->getTextures()) {
                    const TextureType type = texture->getInfo().type;
                    const std::string name = render.model->getTextureTypeName(type);

                    pipeline->setUniform(std::format("PBZ_Texture{}[{}]", name, textureLengths[type]), texture->activate());
                    textureLengths[type]++;
                }

                // load array lengths
                for (std::size_t i = 0; i < TextureTypeMax; i++) {
                    TextureType type = static_cast<TextureType>(i);
                    if (!textureLengths.contains(type)) {
                        continue;
                    }

                    const std::string name = render.model->getTextureTypeName(type);
                    auto a = textureLengths.at(type);

                    pipeline->setUniform(std::format("PBZ_Texture{}Length", name), textureLengths[type]);
                }

                // draw mesh
                pipeline->setUniform("PBZ_Model", render.transform.matrix);

                for (const auto &[mesh, _] : render.model->getMeshs()) {
                    mesh.draw();
                }
            },
        }});
}

bool ShaderDeferredLighting::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    if (!Builtin::MeshRendererScreenQuad::build()) {
        return false;
    }

    return ResourceRegistry<ShaderPipeline>::insert(
        Resource.getIdentifier(),
        {{
            .vertex = {.file = {.path = "resources/shaders/builtin/deferred/lighting.vert"}},
            .tessControl = {},
            .tessEvaluation = {},
            .geometry = {},
            .fragment = {.file = {.path = "resources/shaders/builtin/deferred/lighting.frag"}},
            .compute = {},
            .draw = [](const ShaderPipeline *pipeline, Scene &scene, ObjectID) {
                const auto &points = scene.getComponents<PointLightComponent>();
                const auto &directionals = scene.getComponents<DirectionalLightComponent>();
                const auto &spots = scene.getComponents<SpotLightComponent>();

                pipeline->setUniform<std::uint32_t>("PBZ_PointLightLength", points.size());
                pipeline->setUniform<std::uint32_t>("PBZ_DirectionalLightLength", points.size());
                pipeline->setUniform<std::uint32_t>("PBZ_SpotLightLength", points.size());

                for (std::size_t i = 0; i < points.size(); i++) {
                    const auto &[_, point] = points[i];

                    pipeline->setUniform(std::format("PBZ_PointLight[{}].position", i), point.position);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].ambient", i), point.ambient);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].diffuse", i), point.diffuse);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].specular", i), point.specular);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].constant", i), point.constant);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].linear", i), point.linear);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].quadratic", i), point.quadratic);
                }

                for (std::size_t i = 0; i < directionals.size(); i++) {
                    const auto &[_, direction] = directionals[i];

                    pipeline->setUniform(std::format("PBZ_DirectionalLight[{}].direction", i), direction.direction);
                    pipeline->setUniform(std::format("PBZ_DirectionalLight[{}].ambient", i), direction.ambient);
                    pipeline->setUniform(std::format("PBZ_DirectionalLight[{}].diffuse", i), direction.diffuse);
                    pipeline->setUniform(std::format("PBZ_DirectionalLight[{}].specular", i), direction.specular);
                }

                for (std::size_t i = 0; i < spots.size(); i++) {
                    const auto &[_, spot] = spots[i];

                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].position", i), spot.position);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].direction", i), spot.direction);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].ambient", i), spot.ambient);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].diffuse", i), spot.diffuse);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].specular", i), spot.specular);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].constant", i), spot.constant);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].linear", i), spot.linear);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].quadratic", i), spot.quadratic);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].cutOff", i), glm::cos(spot.cutOff));
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].outerCutOff", i), glm::cos(spot.outerCutOff));
                }

                for (const auto &[mesh, _] : Builtin::MeshRendererScreenQuad::Resource->getMeshs()) {
                    mesh.draw();
                }
            },
        }});
}

} // namespace Builtin

DeferredRenderer::DeferredRenderer(const Info &info, const glm::ivec2 &resolution)
    : m_Info(info),
      m_GBuffer({
          .resolution = resolution,
          .colors = {
              info.gBufferCount,
              {
                  .storage = Framebuffer::Storage::Texture2D,
                  .isDrawn = true,
              },
          },
          .depth = {
              .storage = Framebuffer::Storage::Renderbuffer,
              .hasStencil = true,
          },
      }),
      m_Framebuffer({
          .resolution = resolution,
          .colors = {
              {
                  .storage = Framebuffer::Storage::Texture2D,
                  .isDrawn = true,
              },
          },
          .depth = {
              .storage = Framebuffer::Storage::Renderbuffer,
              .hasStencil = true,
          },
      }) {}

bool DeferredRenderer::build() {
    bool success = true;

    success &= m_GBuffer.build();
    success &= m_Framebuffer.build();

    if (m_Info.passes.geometry.getIdentifier() == Builtin::ShaderDeferredGeometry::Resource.getIdentifier()) {
        success &= Builtin::ShaderDeferredGeometry::build();
    }

    if (m_Info.passes.lighting.getIdentifier() == Builtin::ShaderDeferredLighting::Resource.getIdentifier()) {
        success &= Builtin::ShaderDeferredLighting::build();
    }

    return success;
}

bool DeferredRenderer::destroy() {
    bool success = true;

    success &= m_GBuffer.destroy();
    success &= m_Framebuffer.destroy();

    return success;
}

void DeferredRenderer::tick() const {
    // check for reloads before rendering
    if (!m_Info.passes.geometry->reload()) {
        return;
    }

    if (!m_Info.passes.lighting->reload()) {
        return;
    }

    // geometry pass
    m_GBuffer.bind();
    m_GBuffer.clear();

    // render to gBuffers
    for (const auto &object : m_Objects) {
        const auto [deferred] = m_Scene->getComponent<DeferredRenderComponent>(object);
        render(object);
    }

    // lighting pass
    m_Framebuffer.bind();
    m_Framebuffer.clear();

    m_Info.passes.lighting->bind();

    for (std::size_t i = 0; i < m_GBuffer.getInfo().colors.size(); i++) {
        m_Info.passes.lighting->setUniform(
            std::format("PBZ_GBuffer{}", i),
            m_GBuffer.activate(Framebuffer::Type::Color, i));
    }

    m_Info.passes.lighting->draw(*m_Scene, -1);
    m_Info.passes.lighting->unbind();

    m_Framebuffer.blit(
        m_GBuffer,
        {{0, 0}, m_Framebuffer.getInfo().resolution},
        {{0, 0}, m_Framebuffer.getInfo().resolution},
        Framebuffer::Mask::Depth);

    // forward passes
    for (const auto [object, forward] : m_Scene->getComponents<DeferredRenderComponent::ForwardPass>()) {
        if (!forward.pipeline->reload()) {
            continue;
        }

        forward.pipeline->bind();
        forward.pipeline->draw(*m_Scene, object);
        forward.pipeline->unbind();
    }

    m_Framebuffer.unbind();
}

void DeferredRenderer::render(ObjectID object) const {
    const auto [render] = m_Scene->getComponent<RenderComponent>(object);

    GL::detail::TextureUnits::reset();
    m_Info.passes.geometry->bind();
    m_Info.passes.geometry->draw(*m_Scene, object);
    m_Info.passes.geometry->unbind();
}

void DeferredRenderer::resize(const glm::ivec2 &resolution) {
    m_GBuffer.resize(resolution);
    m_Framebuffer.resize(resolution);
}

const Framebuffer &DeferredRenderer::getFramebuffer() const {
    return m_Framebuffer;
}

const DeferredRenderer::Info &DeferredRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
