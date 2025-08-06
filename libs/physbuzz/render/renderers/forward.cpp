#include "forward.hpp"

#include "../../ecs/scene.hpp"
#include "../lighting.hpp"
#include "../shadow.hpp"

namespace Physbuzz {

namespace Builtin {

bool ShaderForward::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<ShaderPipeline>::insert(
        Resource.getIdentifier(),
        {{
            .vertex = {.file = {.path = "resources/shaders/builtin/forward/forward.vert"}},
            .tessControl = {},
            .tessEvaluation = {},
            .geometry = {},
            .fragment = {.file = {.path = "resources/shaders/builtin/forward/forward.frag"}},
            .compute = {},
            .draw = [](const ShaderPipeline *pipeline, Scene &scene, ObjectID object) {
                const auto [render] = scene.getComponent<RenderComponent>(object);

                const auto &points = scene.getComponents<PointLightComponent>();
                const auto &directionals = scene.getComponents<DirectionalLightComponent>();
                const auto &spots = scene.getComponents<SpotLightComponent>();

                const auto &shadows = scene.getSystem<Shadow>();
                const Shadow::Framebuffers &shadowFramebuffers = shadows->getFramebuffers();
                const float shadowFarPlane = shadows->getInfo().depth;

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
                    const std::string name = render.model->getTextureTypeName(type);

                    if (!textureLengths.contains(type)) {
                        pipeline->setUniform<unsigned int>(std::format("PBZ_Texture{}Length", name), 0);
                    } else {
                        pipeline->setUniform(std::format("PBZ_Texture{}Length", name), textureLengths[type]);
                    }
                }

                pipeline->setUniform<std::uint32_t>("PBZ_PointLightLength", points.size());
                pipeline->setUniform<std::uint32_t>("PBZ_DirectionalLightLength", points.size());
                pipeline->setUniform<std::uint32_t>("PBZ_SpotLightLength", points.size());

                for (std::size_t i = 0; i < points.size(); i++) {
                    const auto &[_, point] = points[i];

                    pipeline->setUniform(std::format("PBZ_PointLight[{}].light.ambient", i), point.ambient);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].light.diffuse", i), point.diffuse);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].light.specular", i), point.specular);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].position", i), point.position);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].constant", i), point.constant);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].linear", i), point.linear);
                    pipeline->setUniform(std::format("PBZ_PointLight[{}].quadratic", i), point.quadratic);
                }

                for (std::size_t i = 0; i < directionals.size(); i++) {
                    const auto &[_, directional] = directionals[i];

                    pipeline->setUniform(std::format("PBZ_DirectionalLight[{}].light.ambient", i), directional.ambient);
                    pipeline->setUniform(std::format("PBZ_DirectionalLight[{}].light.diffuse", i), directional.diffuse);
                    pipeline->setUniform(std::format("PBZ_DirectionalLight[{}].light.specular", i), directional.specular);
                    pipeline->setUniform(std::format("PBZ_DirectionalLight[{}].direction", i), directional.direction);
                    pipeline->setUniform(std::format("PBZ_DirectionalLight[{}].matrix", i), directional.matrix);
                }

                for (std::size_t i = 0; i < spots.size(); i++) {
                    const auto &[_, spot] = spots[i];

                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].light.ambient", i), spot.ambient);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].light.diffuse", i), spot.diffuse);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].light.specular", i), spot.specular);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].position", i), spot.position);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].direction", i), spot.direction);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].constant", i), spot.constant);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].linear", i), spot.linear);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].quadratic", i), spot.quadratic);
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].cutOff", i), glm::cos(spot.cutOff));
                    pipeline->setUniform(std::format("PBZ_SpotLight[{}].outerCutOff", i), glm::cos(spot.outerCutOff));
                }

                // TODO: implement multiple shadowmaps
                pipeline->setUniform("PBZ_DirectionalShadow", shadowFramebuffers.directional.activate(Framebuffer::Type::Depth));
                pipeline->setUniform("PBZ_PointShadow", shadowFramebuffers.point.activate(Framebuffer::Type::Depth));
                pipeline->setUniform("PBZ_ShadowFarPlane", shadows->getInfo().depth);

                pipeline->setUniform("PBZ_Model", render.transform.matrix);

                for (const auto &[mesh, _] : render.model->getMeshs()) {
                    mesh.draw();
                }
            },
        }});
}

} // namespace Builtin

ForwardRenderer::ForwardRenderer(const Info &info, const glm::ivec2 &resolution)
    : m_Info(info),
      m_Output({
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

bool ForwardRenderer::build() {
    bool success = true;

    success &= m_Output.build();
    success &= Builtin::ShaderForward::build();

    return success;
}

bool ForwardRenderer::destroy() {
    return m_Output.destroy();
}

void ForwardRenderer::tick() const {
    m_Output.bind();
    m_Output.clear();

    for (const auto &object : m_Objects) {
        render(object);
    }

    m_Output.unbind();
}

void ForwardRenderer::render(ObjectID object) const {
    // const auto [forward] = m_Scene->getComponent<ForwardRenderComponent>(object);
    //
    // // check for reload before binding
    // if (!forward.pipeline->reload()) {
    //     return;
    // }
    //
    // GL::detail::TextureUnits::reset();
    // forward.pipeline->bind();
    // forward.pipeline->draw(*m_Scene, object);
    // forward.pipeline->unbind();
}

void ForwardRenderer::resize(const glm::ivec2 &resolution) {
    m_Output.resize(resolution);
}

const Framebuffer &ForwardRenderer::getOutput() const {
    return m_Output;
}

const ForwardRenderer::Info &ForwardRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
