#include "game.hpp"

#include <imgui.h>
#include <physbuzz/app/application.hpp>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/layout.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/renderer.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/textures/texture.hpp>

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

struct Camera {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

void Game::build() {
    Physbuzz::App::init();
    Physbuzz::Context::set(this);

    Physbuzz::App::GScene.createSystem<Physbuzz::Transfer>();

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        "test_texture",
        Physbuzz::Texture::Tex2D,
        Physbuzz::ImageFile::Info{
            .file = {
                .path = "resources/textures/floor.png",
            },
        },
        Physbuzz::App::GScene.getSystem<Physbuzz::Transfer>());

    std::shared_ptr<Physbuzz::Window> window = Physbuzz::App::createWindow("main", {}, {1280, 720});

    Physbuzz::App::GScene.createSystem<Physbuzz::PipelineLayoutAllocator>(Physbuzz::PipelineLayoutAllocator::Info{});
    Physbuzz::App::GScene.createSystem<Physbuzz::Renderer>(Physbuzz::Renderer::Info{
        .type = Physbuzz::Renderer::Type::Forward,
        .window = window,
    });

    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::insert(
        "test_shader",
        {{
            .layouts = {
                Physbuzz::Builtin::LayoutRenderer::Resource,
            },
            .description = &TestVertex::Description,
            .shaders = {
                {
                    Physbuzz::RenderPipeline::ShaderStageFlags::eVertex,
                    {
                        .module = {"./spirv/shaders/test/triangle.slang.spv"},
                        .entrypoint = "vertMain",
                    },
                },
                {
                    Physbuzz::RenderPipeline::ShaderStageFlags::eFragment,
                    {
                        .module = {"./spirv/shaders/test/triangle.slang.spv"},
                        .entrypoint = "fragMain",
                    },
                },
            },
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
    auto allocator = Physbuzz::App::GScene.getSystem<Physbuzz::PipelineLayoutAllocator>();

    while (m_IsRunning && !window->shouldClose()) {
        window->poll();
        Physbuzz::App::GScene.tickSystem<Physbuzz::Renderer>();
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
