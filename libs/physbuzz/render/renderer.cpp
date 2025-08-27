#include "renderer.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "camera.hpp"
#include "layout.hpp"
#include "model.hpp"
#include "renderers/defines.hpp"

namespace Physbuzz {

namespace Builtin {

bool MeshRendererScreenQuad::build() {
    if (ResourceRegistry<Model>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<Model>::insert(
        Resource.getIdentifier(),
        {{
            // .meshes = {
            //     {
            //         {
            //             Mesh::Info<Renderer::VertexScreenQuad>{
            //                 .vertices = {
            //                     {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            //                     {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            //                     {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            //                     {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            //                 },
            //                 .indices = {0, 1, 2, 2, 3, 0},
            //             },
            //             {},
            //         },
            //     },
            // },
        }});
}

bool ShaderRendererPassthrough::build() {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
        return true;
    }

    if (!Builtin::MeshRendererScreenQuad::build()) {
        return false;
    }

    // return ResourceRegistry<ShaderPipeline>::insert(
    //     Resource.getIdentifier(),
    //     {{
    //         // .draw = [](const ShaderPipeline *, Scene &, ObjectID) {
    //         //     for (const auto &[mesh, _] : Builtin::MeshRendererScreenQuad::Resource->getMeshs()) {
    //         //         mesh.draw();
    //         //     }
    //         // },
    //     }});

    return true;
}

bool LayoutRenderer::build() {
    if (ResourceRegistry<PipelineLayout>::contains(Resource)) {
        return true;
    }

    bool success = ResourceRegistry<PipelineLayout>::insert(
        Resource,
        {{
            .bindings = {
                {
                    .size = sizeof(Camera),
                    .type = Physbuzz::PipelineLayout::Type::eUniformBuffer,
                    .stage = Physbuzz::PipelineLayout::ShaderStageFlags::eAll,
                },
            },
        }});
    // Resource->bindPipeline(Binding);

    return success;
}

} // namespace Builtin

VertexDescription Renderer::VertexScreenQuad::Description = {{
    .attributes = {
        {
            .format = VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(VertexScreenQuad::position) / sizeof(decltype(VertexScreenQuad::position)::value_type),
            .offset = offsetof(VertexScreenQuad, position),
        },
        {
            .format = VertexDescription::Format::eR32G32Sfloat,
            .size = sizeof(VertexScreenQuad::texCoords) / sizeof(decltype(VertexScreenQuad::texCoords)::value_type),
            .offset = offsetof(VertexScreenQuad, texCoords),
        },
    },
    .size = sizeof(VertexScreenQuad),
    .binding = 0,
}};

Renderer::Renderer(const Info &info)
    : m_Info(info) {}

bool Renderer::build() {
    if (m_Command.pool != nullptr) {
        Logger::WARNING("[Renderer] Cannot build a constructed renderer.");
        return true;
    }

    if (!Builtin::LayoutRenderer::build()) {
        Logger::ERROR("[Renderer] Could not create the builtin pipeline layout.");
        return false;
    }

    if (!Builtin::ShaderRendererPassthrough::build()) {
        return false;
    }

    // create command objects
    m_Command.pool = PBZ_VK_CHECK(App::Device.createCommandPool({
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = App::Indices.graphics,
    }));

    m_Command.buffers = PBZ_VK_CHECK(App::Device.allocateCommandBuffers({
        .commandPool = m_Command.pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = Renderer::Frames::MAX_IN_FLIGHT,
    }));

    // create sync objects
    for (std::size_t i = 0; i < Renderer::Frames::MAX_IN_FLIGHT; i++) {
        m_Semaphores.presentComplete[i] = PBZ_VK_CHECK(App::Device.createSemaphore({}));
        m_Semaphores.renderFinished[i] = PBZ_VK_CHECK(App::Device.createSemaphore({}));
        m_Fences.inFlight[i] = (PBZ_VK_CHECK(App::Device.createFence({
            .flags = vk::FenceCreateFlagBits::eSignaled,
        })));
    }

    // setup resize event
    m_Events = {
        .resize = m_Info.window->addCallback<WindowResizeEvent>([&](const auto &event) {
            resize(event.resolution);
        }),
    };

    // create renderer
    buildSystems();

    // create shadows
    m_Scene->createSystem<Shadow>(m_Info.shadow, m_Info.window->getResolution());

    return true;
}

bool Renderer::destroy() {
    // destroyRenderer(); // System::destroy() should not destroy other systems
    if (m_Command.pool == nullptr) {
        Logger::WARNING("[Renderer] Trying to destroy a destructed renderer");
        return true;
    }

    // destroy sync objects
    for (std::size_t i = 0; i < Renderer::Frames::MAX_IN_FLIGHT; i++) {
        App::Device.destroySemaphore(m_Semaphores.presentComplete[i]);
        App::Device.destroySemaphore(m_Semaphores.renderFinished[i]);

        App::Device.destroyFence(m_Fences.inFlight[i]);
    }

    m_Semaphores.renderFinished.fill(nullptr);
    m_Semaphores.presentComplete.fill(nullptr);
    m_Fences.inFlight.fill(nullptr);

    // destroy command objects
    App::Device.freeCommandBuffers(m_Command.pool, m_Command.buffers.size(), m_Command.buffers.data());
    m_Command.buffers.clear();

    App::Device.destroyCommandPool(m_Command.pool);
    m_Command.pool = nullptr;

    return true;
}

void Renderer::tick() {
    while (vk::Result::eTimeout == App::Device.waitForFences(m_Fences.inFlight[m_Frame.inFlight], vk::True, std::numeric_limits<std::uint64_t>::max())) {
    }

    // fetch the next available swapchain image
    auto [acquireNextImageResult, imageIndex] = App::Device.acquireNextImageKHR(m_Info.window->m_SwapChain, std::numeric_limits<std::uint32_t>::max(), m_Semaphores.presentComplete[m_Frame.inFlight], nullptr);

    switch (acquireNextImageResult) {
    case vk::Result::eSuccess:
    case vk::Result::eSuboptimalKHR:
        break;

    case vk::Result::eErrorOutOfDateKHR:
        m_Info.window->recreateSwapChain();
        return;

    default:
        Logger::CRITICAL("[Renderer] Failed to acquire swap chain image.");
    }

    auto allocator = Physbuzz::App::GScene.getSystem<Physbuzz::PipelineLayoutAllocator>();

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    struct Camera {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    Camera camera = {};

    camera.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    camera.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    camera.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(m_Info.window->getResolution().x) / static_cast<float>(m_Info.window->getResolution().y), 0.1f, 10.0f);
    camera.proj[1][1] *= -1;

    allocator->update<Camera>(
        {"camera"},
        0,
        {
            camera,
        });

    // const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
    //
    // allocator->update<Builtin::LayoutRenderer::Camera>(
    //     Builtin::LayoutRenderer::Handle,
    //     Builtin::LayoutRenderer::Camera::Binding,
    //     {{
    //         .position = camera.getInfo().view.position,
    //         .view = camera.getView(),
    //         .projection = camera.getProjection(),
    //     }});

    App::Device.resetFences(m_Fences.inFlight[m_Frame.inFlight]);
    m_Command.buffers[m_Frame.inFlight].reset();

    {
        vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
        vk::Result result = m_Command.buffers[m_Frame.inFlight].begin(commandBufferBeginInfo);

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Renderer] Rendering begin failed ({})", vk::to_string(result));
            return;
        }
    }

    // transition image
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
            .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .image = m_Info.window->m_SwapChainImages[imageIndex],
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        vk::DependencyInfo dependencyInfo = {
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        m_Command.buffers[m_Frame.inFlight].pipelineBarrier2(dependencyInfo);
    }

    // setup attachments
    std::vector<vk::RenderingAttachmentInfo> attachments = {
        {
            .imageView = m_Info.window->m_SwapChainImageViews[imageIndex],
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
        },
    };

    // setup rendering
    glm::ivec2 resolution = m_Info.window->getResolution();

    m_Command.buffers[m_Frame.inFlight].beginRendering({
        .renderArea = {
            .offset = {0, 0},
            .extent = {static_cast<std::uint32_t>(resolution.x), static_cast<std::uint32_t>(resolution.y)},
        },
        .layerCount = 1,
        .colorAttachmentCount = static_cast<std::uint32_t>(attachments.size()),
        .pColorAttachments = attachments.data(),
    });

    switch (m_Info.type) {
    case Type::Deferred:
        m_Scene->tickSystem<DeferredRenderer>(m_Command.buffers[m_Frame.inFlight]);
        break;

    case Type::Forward:
        m_Scene->tickSystem<ForwardRenderer>(m_Command.buffers[m_Frame.inFlight]);
        break;

    default:
        Logger::ERROR("[Renderer] Unknown renderer type provided");
        return;
    }

    m_Command.buffers[m_Frame.inFlight].endRendering();

    // transition image
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
            .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .newLayout = vk::ImageLayout::eUndefined,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .image = m_Info.window->m_SwapChainImages[imageIndex],
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        vk::DependencyInfo dependencyInfo = {
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        m_Command.buffers[m_Frame.inFlight].pipelineBarrier2(dependencyInfo);
    }

    {
        vk::Result result = m_Command.buffers[m_Frame.inFlight].end();

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Renderer] Rendering end failed ({})", vk::to_string(result));
            return;
        }
    }

    // const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
    // Builtin::UniformRendererCamera::Resource->update({
    //     .position = camera.getInfo().view.position,
    //     .view = camera.getView(),
    //     .projection = camera.getProjection(),
    // });

    {
        vk::PipelineStageFlags waitDestinationStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        const vk::SubmitInfo submitInfo = {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_Semaphores.presentComplete[m_Frame.inFlight],
            .pWaitDstStageMask = &waitDestinationStageMask,
            .commandBufferCount = 1,
            .pCommandBuffers = &m_Command.buffers[m_Frame.inFlight],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_Semaphores.renderFinished[m_Frame.inFlight],
        };

        vk::Result result = App::Queues.graphics.submit(submitInfo, m_Fences.inFlight[m_Frame.inFlight]);

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Renderer] Queue submission failed ({})", vk::to_string(result));
            return;
        }
    }

    {
        vk::Result result = App::Queues.present.presentKHR({
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_Semaphores.renderFinished[m_Frame.inFlight],
            .swapchainCount = 1,
            .pSwapchains = &m_Info.window->m_SwapChain,
            .pImageIndices = &imageIndex,
        });

        switch (result) {
        case vk::Result::eSuccess:
            break;
        case vk::Result::eErrorOutOfDateKHR:
        case vk::Result::eSuboptimalKHR:
            m_Info.window->recreateSwapChain();
            break;
        default:
            Logger::ERROR("[Renderer] Failed to present swapchain image! ({})", vk::to_string(result));
            return;
        }

        if (m_Info.window->m_FramebufferResized) {
            m_Info.window->m_FramebufferResized = false;
            m_Info.window->recreateSwapChain();
        }
    }

    m_Frame.inFlight = (m_Frame.inFlight + 1) % Renderer::Frames::MAX_IN_FLIGHT;
}

void Renderer::resize(const glm::ivec2 &resolution) {
    // if (!m_Scene->containsComponent<CameraComponent>(m_Info.camera)) {
    //     Logger::ERROR("[Renderer] No camera attached to object {}", m_Info.camera);
    //     return;
    // }
    //
    // const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);

    m_Scene->getSystem<Shadow>()->resize(resolution);
    getRenderer()->resize(resolution);
    // camera.resize(resolution);
}

void Renderer::setType(const Type &type) {
    destroySystems();
    m_Info.type = type;
    buildSystems();
}

const Renderer::Type &Renderer::getType() {
    return m_Info.type;
}

const Framebuffer &Renderer::getFramebuffer() const {
    return getRenderer()->getOutput();
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

bool Renderer::buildSystems() {
    switch (m_Info.type) {
    case Type::Deferred:
        m_Scene->createSystem<DeferredRenderer>(m_Info.deferred, m_Info.window->getResolution());
        break;

    case Type::Forward:
        m_Scene->createSystem<ForwardRenderer>(m_Info.forward, m_Info.window->getResolution());
        break;
    }

    return false;
}

bool Renderer::destroySystems() {
    switch (m_Info.type) {
    case Type::Deferred:
        return m_Scene->eraseSystem<DeferredRenderer>();

    case Type::Forward:
        return m_Scene->eraseSystem<ForwardRenderer>();
    }

    return false;
}

} // namespace Physbuzz
