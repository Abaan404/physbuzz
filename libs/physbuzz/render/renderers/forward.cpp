#include "forward.hpp"

#include "../../ecs/scene.hpp"
#include "../gl/units.hpp"

namespace Physbuzz {

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
    return m_Output.build();
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
    const auto [forward] = m_Scene->getComponent<ForwardRenderComponent>(object);

    // check for reload before binding
    if (!forward.pipeline->reload()) {
        return;
    }

    GL::detail::TextureUnits::reset();
    forward.pipeline->bind();
    forward.pipeline->draw(*m_Scene, object);
    forward.pipeline->unbind();
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
