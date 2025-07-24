#include "renderer.hpp"

#include "../ecs/scene.hpp"
#include "camera.hpp"
#include "gl/units.hpp"

namespace Physbuzz {

namespace Builtin {

bool VertexRendererScreenQuad::build() {
    if (ResourceRegistry<VertexAttribute>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<VertexAttribute>::insert(
        Resource.getIdentifier(),
        {{
            .attributes = {
                {
                    .type = Types::Float,
                    .size = sizeof(Format::position) / sizeof(decltype(Format::position)::value_type),
                    .offset = offsetof(Format, position),
                },
                {
                    .type = Types::Float,
                    .size = sizeof(Format::texCoords) / sizeof(decltype(Format::texCoords)::value_type),
                    .offset = offsetof(Format, texCoords),
                },
            },
            .size = sizeof(Format),
        }});
}

bool MeshRendererScreenQuad::build() {
    if (ResourceRegistry<Model>::contains(Resource.getIdentifier())) {
        return true;
    }

    if (!VertexRendererScreenQuad::build()) {
        return false;
    }

    return ResourceRegistry<Model>::insert(
        Resource.getIdentifier(),
        {{
            .meshes = {
                {
                    {
                        Mesh::Info<VertexRendererScreenQuad::Format>{
                            .attribute = {VertexRendererScreenQuad::Resource.getIdentifier()},
                            .vertices = {
                                {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
                                {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
                                {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
                                {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                            },
                            .indices = {0, 1, 2, 2, 3, 0},
                        },
                        {},
                    },
                },
            },
        }});
}

bool ShaderRendererPassthrough::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    if (!Builtin::MeshRendererScreenQuad::build()) {
        return false;
    }

    return ResourceRegistry<ShaderPipeline>::insert(
        Resource.getIdentifier(),
        {{
            .vertex = {.file = {.path = "resources/shaders/builtin/passthrough/passthrough.vert"}},
            .tessControl = {},
            .tessEvaluation = {},
            .geometry = {},
            .fragment = {.file = {.path = "resources/shaders/builtin/passthrough/passthrough.frag"}},
            .compute = {},
            .draw = [](const ShaderPipeline *, Scene &, ObjectID) {
                for (const auto &[mesh, _] : Builtin::MeshRendererScreenQuad::Resource->getMeshs()) {
                    mesh.draw();
                }
            },
        }});
}

bool UniformRendererCamera::build() {
    if (ResourceRegistry<UniformBuffer<Format>>::contains(Resource.getIdentifier())) {
        return true;
    }

    bool success = ResourceRegistry<UniformBuffer<Format>>::insert(Resource.getIdentifier(), {});
    Resource->bindPipeline(Binding);

    return success;
}

} // namespace Builtin

Renderer::Renderer(const Info &info)
    : m_Info(info) {}

bool Renderer::build() {
    if (!Builtin::UniformRendererCamera::build()) {
        Logger::ERROR("[Renderer] Could not create a constant camera buffer.");
        return false;
    }

    if (!Builtin::ShaderRendererPassthrough::build()) {
        return false;
    }

    // create renderer
    buildRenderer();

    return true;
}

bool Renderer::destroy() {
    // destroyRenderer(); // System::destroy() should not destroy other systems
    return true;
}

void Renderer::tick() {
    if (!m_Scene->containsComponent<CameraComponent>(m_Info.camera)) {
        Logger::ERROR("[DeferredRenderer] No camera attached to object {}", m_Info.camera);
        return;
    }

    const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
    Builtin::UniformRendererCamera::Resource->update({
        .position = camera.getInfo().view.position,
        .view = camera.getView(),
        .projection = camera.getProjection(),
    });

    switch (m_Info.type) {
    case Type::Deferred:
        m_Scene->tickSystem<DeferredRenderer>();
        break;

    case Type::Forward:
        m_Scene->tickSystem<ForwardRenderer>();
        break;

    default:
        Logger::ERROR("[Renderer] Unknown renderer type provided");
        return;
    }

    // disable depth testing for screenquads
    bool depthTest = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
    glDisable(GL_DEPTH_TEST);

    // get renderer output buffer
    GL::detail::TextureUnits::reset();
    const Framebuffer &framebuffer = getRenderer()->getFramebuffer();
    GLint screenUnit = framebuffer.activate(Framebuffer::Type::Color);

    // add any post processing effects
    if (m_Info.postProcessing.size() > 0) {
        framebuffer.bind();

        for (const auto &postProcessing : m_Info.postProcessing) {
            if (!postProcessing->reload()) {
                continue;
            }

            postProcessing->bind();
            postProcessing->setUniform("PBZ_Framebuffer", screenUnit);
            postProcessing->draw(*m_Scene, -1);
            postProcessing->unbind();
        }

        framebuffer.unbind();
    }

    // draw to screen if requested
    if (m_Info.passthrough && Builtin::ShaderRendererPassthrough::Resource->reload()) {
        Builtin::ShaderRendererPassthrough::Resource->bind();
        Builtin::ShaderRendererPassthrough::Resource->setUniform("PBZ_Framebuffer", screenUnit);
        Builtin::ShaderRendererPassthrough::Resource->draw(*m_Scene, -1);
        Builtin::ShaderRendererPassthrough::Resource->unbind();
    }

    // restore depth testing
    if (depthTest) {
        glEnable(GL_DEPTH_TEST);
    }
}

void Renderer::resize(const glm::ivec2 &resolution) {
    if (!m_Scene->containsComponent<CameraComponent>(m_Info.camera)) {
        Logger::ERROR("[Renderer] No camera attached to object {}", m_Info.camera);
        return;
    }

    const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);

    getRenderer()->resize(resolution);
    camera.resize(resolution);
}

void Renderer::setType(const Type &type) {
    destroyRenderer();
    m_Info.type = type;
    buildRenderer();
}

const Renderer::Type &Renderer::getType() {
    return m_Info.type;
}

const Framebuffer &Renderer::getFramebuffer() const {
    return getRenderer()->getFramebuffer();
}

const Renderer::Info &Renderer::getInfo() const {
    return m_Info;
}

std::shared_ptr<IRenderer> Renderer::getRenderer() const {
    switch (m_Info.type) {
    case Type::Deferred:
        return m_Scene->getSystem<DeferredRenderer>();

    case Type::Forward:
        return m_Scene->getSystem<ForwardRenderer>();
    }

    return nullptr;
}

bool Renderer::buildRenderer() {
    switch (m_Info.type) {
    case Type::Deferred:
        m_Scene->createSystem<DeferredRenderer>(m_Info.deferred, m_Info.resolution);
        break;

    case Type::Forward:
        m_Scene->createSystem<ForwardRenderer>(m_Info.forward, m_Info.resolution);
        break;
    }

    return false;
}

bool Renderer::destroyRenderer() {
    switch (m_Info.type) {
    case Type::Deferred:
        return m_Scene->eraseSystem<DeferredRenderer>();

    case Type::Forward:
        return m_Scene->eraseSystem<ForwardRenderer>();
    }

    return false;
}

} // namespace Physbuzz
