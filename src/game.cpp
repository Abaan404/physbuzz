#include "game.hpp"

#include <imgui.h>
#include <physbuzz/app/application.hpp>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/renderer.hpp>
#include <physbuzz/render/shaders.hpp>

void Game::build() {
    Physbuzz::App::init();
    Physbuzz::Context::set(this);

    std::shared_ptr<Physbuzz::Window> window = Physbuzz::App::createWindow("main", {}, {1280, 720});

    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::insert(
        "test_shader",
        {{
            .module = {"./spirv/shaders/test/triangle.slang.spv"},
            .shaders = {
                {Physbuzz::ShaderPipeline::Stage::eVertex, {"vertMain"}},
                {Physbuzz::ShaderPipeline::Stage::eFragment, {"fragMain"}},
            },
            .draw = [](const Physbuzz::ShaderPipeline *, const Physbuzz::RenderCommand &command, Physbuzz::Scene &, Physbuzz::ObjectID) {
                std::shared_ptr<Physbuzz::Window> window = Physbuzz::App::getWindow("main");
                glm::ivec2 resolution = window->getResolution();

                command.buffers[command.frameInFlight].setViewport(0, vk::Viewport(0.0f, 0.0f, resolution.x, resolution.y, 0.0f, 1.0f));
                command.buffers[command.frameInFlight].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), {static_cast<uint32_t>(resolution.x), static_cast<uint32_t>(resolution.y)}));
                command.buffers[command.frameInFlight].draw(3, 1, 0, 0);
            },
        }});

    Physbuzz::RenderComponent render = {
        .transform = {},
        .model = {""},
    };

    Physbuzz::ForwardRenderComponent forward = {
        .pipeline = {"test_shader"},
    };

    Physbuzz::ObjectID obj = Physbuzz::App::GlobalScene.createObject();

    Physbuzz::App::GlobalScene.setComponent(obj, render, forward);
    Physbuzz::App::GlobalScene.createSystem<Physbuzz::Renderer>(Physbuzz::Renderer::Info{
        .type = Physbuzz::Renderer::Type::Forward,
        .window = window,
    });
}

void Game::rebuild() {
}

void Game::loop() {
    m_IsRunning = true;

    const std::shared_ptr<Physbuzz::Window> &window = Physbuzz::App::getWindow("main");

    while (m_IsRunning && !window->shouldClose()) {
        window->poll();
        Physbuzz::App::GlobalScene.tickSystem<Physbuzz::Renderer>();
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
