#include "objectpicker.hpp"

#include "../../game.hpp"
#include "../../objects/builder.hpp"
#include "../../objects/circle.hpp"
#include "../../objects/quad.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <physbuzz/context.hpp>

ObjectPicker::ObjectPicker() {
    Quad quad = {
        .transform = {
            .position = {m_PreviewSize.x / 2.0f, m_PreviewSize.y / 2.0f, 0.0f},
        },
        .quad = {
            .width = m_PreviewSize.x,
            .height = m_PreviewSize.y,
        },
        .isRenderable = true,
    };

    Circle circle = {
        .transform = {
            .position = {m_PreviewSize.x / 2.0f, m_PreviewSize.y / 2.0f, 0.0f},
        },
        .circle = {
            .radius = glm::min(m_PreviewSize.x, m_PreviewSize.y) / 2.0f,
        },
        .isRenderable = true,
    };

    Game *game = Physbuzz::Context::get<Game>();
    ObjectBuilder builder = ObjectBuilder(m_Scene);

    builder.create(circle);
    builder.create(quad);

    for (auto &object : m_Scene.getObjects()) {
        PickableComponent pickable = {
            .selected = false,
            .framebuffer = Physbuzz::Framebuffer({m_PreviewSize.x, m_PreviewSize.y}),
        };

        pickable.framebuffer.build();
        object.setComponent(pickable);
    }

    // set orthographic projection for preview
    m_Camera.setOrthographic({m_PreviewSize.x, m_PreviewSize.y});
}

ObjectPicker::~ObjectPicker() {
    std::vector<PickableComponent> &pickables = m_Scene.getComponents<PickableComponent>();
    for (auto &pickable : pickables) {
        pickable.framebuffer.destroy();
    }
}

void ObjectPicker::draw() {
    const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(128, 256), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags windowFlags = 0;
    if (!ImGui::Begin("ShapePicker", &show, windowFlags)) {
        ImGui::End();
        return;
    }

    static glm::vec4 bgColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    Game *game = Physbuzz::Context::get<Game>();
    game->renderer.activeCamera = &m_Camera;

    // TODO buttons
    for (auto &object : m_Scene.getObjects()) {
        PickableComponent &pickable = object.getComponent<PickableComponent>();

        // render to framebuffer
        game->renderer.target(&pickable.framebuffer);
        game->renderer.clear(bgColor);
        game->renderer.render(object, game->resources);

        // imgui fuckery
        ImGui::Image((void *)(intptr_t)pickable.framebuffer.getColor(), m_PreviewSize);
    }

    // release target
    game->renderer.target(nullptr);
    game->renderer.activeCamera = &game->player.camera;

    ImGui::End();
}
