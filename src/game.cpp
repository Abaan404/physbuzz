#include "game.hpp"

#include "objects/player.hpp"
#include "physbuzz/misc/clock.hpp"
#include "physbuzz/physics/dynamics.hpp"
#include "ui/handler.hpp"
#include <physbuzz/app/application.hpp>
#include <physbuzz/compat/imgui/imgui_impl_physbuzz.hpp>
#include <physbuzz/events/window.hpp>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/layout.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/renderer.hpp>
#include <physbuzz/render/renderers/forward.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/window/bindings.hpp>

struct TestVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static Physbuzz::VertexDescription Description;
};

Physbuzz::VertexDescription TestVertex::Description = {{
    .attributes = {
        {
            .format = Physbuzz::VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(TestVertex::position) / sizeof(decltype(TestVertex::position)::value_type),
            .offset = offsetof(TestVertex, position),
        },
        {
            .format = Physbuzz::VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(TestVertex::color) / sizeof(decltype(TestVertex::color)::value_type),
            .offset = offsetof(TestVertex, color),
        },
        {
            .format = Physbuzz::VertexDescription::Format::eR32G32Sfloat,
            .size = sizeof(TestVertex::texCoord) / sizeof(decltype(TestVertex::texCoord)::value_type),
            .offset = offsetof(TestVertex, texCoord),
        },
    },
    .size = sizeof(TestVertex),
    .binding = 0,
}};

void Game::build() {
    Physbuzz::App::init();
    Physbuzz::Context::set(this);

    std::shared_ptr<Physbuzz::Window> window = Physbuzz::App::createWindow("main", {}, {1280, 720});

    // track cursor captures
    window->addCallback<Physbuzz::MousePositionEvent>([](const Physbuzz::MousePositionEvent &event) {
        static glm::vec2 lastPosition = event.window->getResolution() >> 1u;

        std::shared_ptr<Physbuzz::Renderer> renderer = Physbuzz::App::GScene.getSystem<Physbuzz::Renderer>();
        const auto [player, camera, flashlight] = Physbuzz::App::GScene.getComponent<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>(renderer->getInfo().camera);

        glm::vec2 offset = (static_cast<glm::vec2>(event.position) - lastPosition) * player.sensitivity;
        lastPosition = event.position;

        // if (player.captureMouse || Physbuzz::App::GScene.getSystem<InterfaceManager>()->draw) {
        if (player.captureMouse) {
            return;
        }

        event.window->setCursorCapture(true);

        const Physbuzz::CameraComponent::Info &info = camera.getInfo();
        glm::quat pitch = glm::angleAxis(glm::radians(offset.x), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::quat yaw = glm::angleAxis(glm::radians(offset.y), glm::cross(camera.getUp(), camera.getFacing()));

        camera.setOrientation(pitch * yaw * info.view.orientation);
        flashlight.direction = camera.getFacing();
    });

    // change prespective camera fov when scrolling
    window->addCallback<Physbuzz::MouseScrollEvent>([&](const Physbuzz::MouseScrollEvent &event) {
        std::shared_ptr<Physbuzz::Renderer> renderer = Physbuzz::App::GScene.getSystem<Physbuzz::Renderer>();
        const auto [player, camera] = Physbuzz::App::GScene.getComponent<PlayerComponent, Physbuzz::CameraComponent>(renderer->getInfo().camera);

        Physbuzz::CameraComponent::Info info = camera.getInfo();

        if (player.captureMouse || ImGui::GetIO().WantCaptureMouse || info.projection != Physbuzz::CameraComponent::Projection::Perspective) {
            return;
        }

        info.perspective.fovy = glm::clamp(info.perspective.fovy + glm::radians<float>(event.offset.y), glm::radians(30.0f), glm::radians(135.0f));

        camera.update(info);
    });

    Physbuzz::ObjectID playerObject = Physbuzz::App::GScene.createObject();

    Physbuzz::App::GScene.createSystem<Physbuzz::Transfer>();
    Physbuzz::App::GScene.createSystem<Physbuzz::PipelineLayoutAllocator>(Physbuzz::PipelineLayoutAllocator::Info{});
    Physbuzz::App::GScene.createSystem<Physbuzz::Renderer>(Physbuzz::Renderer::Info{
        .camera = playerObject,
        .window = window,
    });

    Physbuzz::App::GScene.createSystem<Physbuzz::ImGuiRenderer>();

    Physbuzz::App::GScene.getSystem<Physbuzz::Renderer>()->setRenderPasses({
        Physbuzz::App::GScene.createSystem<Physbuzz::ForwardRenderer>(),
    });

    Physbuzz::App::GScene.createSystem<Physbuzz::Bindings>(window);
    Physbuzz::App::GScene.createSystem<Physbuzz::Clock>();
    Physbuzz::App::GScene.createSystem<Physbuzz::Dynamics>(1.0f);
    Physbuzz::App::GScene.createSystem<InterfaceManager>();

    Physbuzz::ResourceRegistry<Physbuzz::PipelineLayout>::insert(
        "test_layout",
        Physbuzz::PipelineLayout::Info{
            .bindings = {
                {
                    .type = Physbuzz::PipelineLayout::Type::eCombinedImageSampler,
                    .stage = Physbuzz::PipelineLayout::ShaderStageFlags::eAll,
                },
            },
        });

    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::insert(
        "test_shader",
        {{
            .layouts = {
                Physbuzz::Builtin::LayoutRenderer::Resource,
                {"test_layout"},
            },
            .description = &TestVertex::Description,
            .module = "test/triangle",
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Model>::insert(
        "test_model",
        {{
            .meshes = {
                {
                    Physbuzz::Mesh::Info<TestVertex>{
                        .vertices = {
                            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
                            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

                            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
                            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
                        },
                        .indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4}},
                    {},
                },
            },
        }},
        Physbuzz::App::GScene.getSystem<Physbuzz::Transfer>());

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        "test_texture",
        Physbuzz::Texture::Tex2D,
        Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/floor.png"}},
        Physbuzz::App::GScene.getSystem<Physbuzz::Transfer>());

    Physbuzz::App::GScene.getSystem<Physbuzz::PipelineLayoutAllocator>()->attach({"test_layout"}, 0, Physbuzz::Resource<Physbuzz::Texture>{"test_texture"});

    Physbuzz::RenderComponent render = {
        .transform = {},
        .model = {"test_model"},
    };

    render.transform.update();

    Physbuzz::ForwardRenderComponent forward = {
        .pipeline = {"test_shader"},
    };

    Physbuzz::ObjectID testObject = Physbuzz::App::GScene.createObject();
    Physbuzz::App::GScene.setComponent(testObject, render, forward);

    Player player = {
        .camera = {{
            .projection = Physbuzz::CameraComponent::Projection::Perspective,
            .orthographic = {},
            .perspective = {
                .fovy = glm::radians(45.0f),
            },
            .depth = {
                .near = 1.0f,
                .far = 10000.0f,
            },
            .view = {
                .position = {0.0f, 0.0f, 2.0f},
            },
            .resolution = window->getResolution(),
        }},
        .player = {
            .speed = 0.01f,
        },
    };

    ObjectBuilder::create(Physbuzz::App::GScene, playerObject, player);
}

void Game::rebuild() {
}

void Game::loop() {
    m_IsRunning = true;

    const std::shared_ptr<Physbuzz::Window> &window = Physbuzz::App::getWindow("main");

    while (m_IsRunning && !window->shouldClose()) {
        Physbuzz::App::GScene.tickSystem<Physbuzz::Clock>();
        Physbuzz::App::GScene.tickSystem<Physbuzz::Dynamics>();
        Physbuzz::App::GScene.tickSystem<Physbuzz::Bindings>();
        Physbuzz::App::GScene.tickSystem<InterfaceManager, Physbuzz::Renderer>();
    }
}

void Game::destroy() {
    m_IsRunning = false;

    Physbuzz::App::quit();
}

int main() {
    Game game = Game();

    game.build();
    game.loop();
    game.destroy();

    return 0;
}
