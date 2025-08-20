#include "game.hpp"

#include <imgui.h>
#include <physbuzz/app/application.hpp>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/renderer.hpp>
#include <physbuzz/render/shaders.hpp>

struct TestVertex {
    glm::vec2 position;
    glm::vec3 color;

    static Physbuzz::VertexDescription Description;
};

Physbuzz::VertexDescription TestVertex::Description = {{
    .attributes = {
        {
            .format = Physbuzz::VertexDescription::Format::eR32G32Sfloat,
            .size = sizeof(TestVertex::position) / sizeof(decltype(TestVertex::position)::value_type),
            .offset = offsetof(TestVertex, position),
        },
        {
            .format = Physbuzz::VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(TestVertex::color) / sizeof(decltype(TestVertex::color)::value_type),
            .offset = offsetof(TestVertex, color),
        },
    },
    .size = sizeof(TestVertex),
    .binding = 0,
}};

void Game::build() {
    Physbuzz::App::init();
    Physbuzz::Context::set(this);

    std::shared_ptr<Physbuzz::Window> window = Physbuzz::App::createWindow("main", {}, {1280, 720});

    Physbuzz::App::GScene.createSystem<Physbuzz::Transfer>();
    Physbuzz::App::GScene.createSystem<Physbuzz::Renderer>(Physbuzz::Renderer::Info{
        .type = Physbuzz::Renderer::Type::Forward,
        .window = window,
    });

    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::insert(
        "test_shader",
        {{
            .module = {"./spirv/shaders/test/triangle.slang.spv"},
            .description = &TestVertex::Description,
            .shaders = {
                {Physbuzz::ShaderPipeline::Stage::eVertex, {"vertMain"}},
                {Physbuzz::ShaderPipeline::Stage::eFragment, {"fragMain"}},
            },
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Model>::insert(
        "test_model",
        {{
            .meshes = {
                {
                    Physbuzz::Mesh::Info<TestVertex>{
                        .transfer = Physbuzz::App::GScene.getSystem<Physbuzz::Transfer>(),
                        .vertices = {
                            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}},
                        .indices = {0, 1, 2, 2, 3, 0},
                    },
                    {},
                },
            },
        }});

    Physbuzz::RenderComponent render = {
        .transform = {},
        .model = {"test_model"},
    };

    Physbuzz::ForwardRenderComponent forward = {
        .pipeline = {"test_shader"},
    };

    Physbuzz::ObjectID obj = Physbuzz::App::GScene.createObject();

    Physbuzz::App::GScene.setComponent(obj, render, forward);
}

void Game::rebuild() {
}

void Game::loop() {
    m_IsRunning = true;

    const std::shared_ptr<Physbuzz::Window> &window = Physbuzz::App::getWindow("main");

    while (m_IsRunning && !window->shouldClose()) {
        window->poll();
        Physbuzz::App::GScene.tickSystem<Physbuzz::Renderer>();
        Physbuzz::App::GScene.tickSystem<Physbuzz::Transfer>();
    }
}

void Game::destroy() {
    m_IsRunning = false;

    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::erase("test_shader");

    Physbuzz::App::quit();
}

int main() {
    Game game = Game();

    game.build();
    game.loop();
    game.destroy();

    return 0;
}
