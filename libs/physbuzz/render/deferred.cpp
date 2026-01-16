#include "deferred.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineDeferred::build() {
    return false;
}

} // namespace Builtin

DeferredRenderer::DeferredRenderer(const Info &info)
    : m_Info(info) {}

bool DeferredRenderer::build() {
    bool success = true;

    if (m_Info.passes.geometry == Builtin::RenderPipelineDeferred::ResourceGeometry || m_Info.passes.lighting == Builtin::RenderPipelineDeferred::ResourceLighting) {
        if (!Builtin::RenderPipelineDeferred::build()) {
            Logger::ERROR("[Renderer] Could not build forward shader pipeline.");
            return false;
        }
    }

    return success;
}

bool DeferredRenderer::destroy() {
    bool success = true;

    return success;
}

void DeferredRenderer::render(const RenderContext &) {
    // // check for reloads before rendering
    // if (!m_Info.passes.geometry->reload()) {
    //     return;
    // }
    //
    // if (!m_Info.passes.lighting->reload()) {
    //     return;
    // }
    //
    // // geometry pass
    // m_Framebuffers.gBuffer.bind();
    // m_Framebuffers.gBuffer.clear();
    //
    // // render to gBuffers
    // for (const auto &object : m_Objects) {
    //     render(object);
    // }
    //
    // // lighting pass
    // m_Framebuffers.output.bind();
    // m_Framebuffers.output.clear();
    //
    // m_Info.passes.lighting->bind();
    //
    // for (std::size_t i = 0; i < m_Framebuffers.gBuffer.getInfo().colors.size(); i++) {
    //     m_Info.passes.lighting->setUniform(
    //         std::format("PBZ_GBuffer{}", i),
    //         m_Framebuffers.gBuffer.activate(Framebuffer::Type::Color, i));
    // }
    //
    // m_Info.passes.lighting->draw(*m_Scene, -1);
    // m_Info.passes.lighting->unbind();
    //
    // m_Framebuffers.output.blit(
    //     m_Framebuffers.gBuffer,
    //     {{0, 0}, m_Framebuffers.output.getInfo().resolution},
    //     {{0, 0}, m_Framebuffers.output.getInfo().resolution},
    //     Framebuffer::Mask::Depth);
    //
    // // forward passes
    // for (const auto [object, forward] : m_Scene->getComponents<DeferredRenderComponent::ForwardPass>()) {
    //     if (!forward.pipeline->reload()) {
    //         continue;
    //     }
    //
    //     forward.pipeline->bind();
    //     forward.pipeline->draw(*m_Scene, object);
    //     forward.pipeline->unbind();
    // }
    //
    // m_Framebuffers.output.unbind();
}

// void DeferredRenderer::render(ObjectID object) const {
// const auto [render] = m_Scene->getComponent<RenderComponent>(object);
//
// GL::detail::TextureUnits::reset();
// m_Info.passes.geometry->bind();
// m_Info.passes.geometry->draw(*m_Scene, object);
// m_Info.passes.geometry->unbind();
// }

const DeferredRenderer::Info &DeferredRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
