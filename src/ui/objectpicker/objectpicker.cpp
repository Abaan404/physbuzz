#include "objectpicker.hpp"

#include "../../game.hpp"
#include "../../objects/builder.hpp"
#include "../../objects/circle.hpp"
#include "../../objects/quad.hpp"
#include "../../renderer.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <physbuzz/misc/context.hpp>

ObjectPicker::ObjectPicker() {
    Quad quad = {
        .model = {
            .position = {m_PreviewSize.x / 2.0f, m_PreviewSize.y / 2.0f, 0.0f},
        },
        .quad = {
            .width = m_PreviewSize.x,
            .height = m_PreviewSize.y,
        },
        .isRenderable = true,
    };

    Circle circle = {
        .model = {
            .position = {m_PreviewSize.x / 2.0f, m_PreviewSize.y / 2.0f, 0.0f},
        },
        .circle = {
            .radius = glm::min(m_PreviewSize.x, m_PreviewSize.y) / 2.0f,
        },
        .isRenderable = true,
    };

    ObjectBuilder builder = ObjectBuilder(&m_Scene);

    builder.create(circle);
    builder.create(quad);

    for (const auto &object : m_Scene.getObjects()) {
        PickableComponent pickable = {
            .selected = false,
            .framebuffer = Physbuzz::Framebuffer({m_PreviewSize.x, m_PreviewSize.y}),
        };

        pickable.framebuffer.build();
        m_Scene.setComponent(object, pickable);
    }

    // set orthographic projection for preview
    m_Camera.setOrthographic({m_PreviewSize.x, m_PreviewSize.y});
}

ObjectPicker::~ObjectPicker() {
    for (const auto &object : m_Scene.getObjects()) {
        if (m_Scene.containsComponent<PickableComponent>(object)) {
            m_Scene.getComponent<PickableComponent>(object).framebuffer.destroy();
        }
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

    const std::shared_ptr<Renderer> renderer = game->scene.getSystem<Renderer>();

    renderer->activeCamera = &m_Camera;

    // TODO buttons
    for (const auto &object : m_Scene.getObjects()) {
        PickableComponent &pickable = m_Scene.getComponent<PickableComponent>(object);

        // render to framebuffer
        renderer->target(&pickable.framebuffer);
        renderer->clear(bgColor);
        renderer->render(m_Scene, object);

        // imgui fuckery
        ImGui::Image((void *)(intptr_t)pickable.framebuffer.getColor(), m_PreviewSize);
    }

    // release target
    renderer->target(nullptr);
    renderer->activeCamera = &game->player.camera;

    ImGui::End();
}
