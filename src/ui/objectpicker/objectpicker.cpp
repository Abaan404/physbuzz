#include "objectpicker.hpp"

#include "../../game.hpp"
#include "../../objects/circle.hpp"
#include "../../objects/objects.hpp"
#include "../../objects/quad.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <physbuzz/context.hpp>

ObjectPicker::ObjectPicker() {
    QuadInfo quadInfo = {
        .transform = {
            .position = {m_PreviewSize.x >> 1, m_PreviewSize.y >> 1, 0.0f},
        },
        .quad = {
            .width = static_cast<float>(m_PreviewSize.x),
            .height = static_cast<float>(m_PreviewSize.y),
        },
        .isRenderable = true,
    };

    CircleInfo circleInfo = {
        .transform = {
            .position = {m_PreviewSize.x >> 1, m_PreviewSize.y >> 1, 0.0f},
        },
        .circle = {
            .radius = static_cast<float>(glm::min(m_PreviewSize.x, m_PreviewSize.y) >> 1),
        },
        .isRenderable = true,
    };

    ObjectBuilder<CircleInfo>::build(m_Scene, circleInfo);
    ObjectBuilder<QuadInfo>::build(m_Scene, quadInfo);

    for (auto &object : m_Scene.getObjects()) {
        PickableComponent pickable = {
            .selected = false,
            .framebuffer = Physbuzz::Framebuffer(m_PreviewSize),
        };

        pickable.framebuffer.build();
        object.setComponent(pickable);
    }
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

    ImVec2 size = {(float)m_PreviewSize.x, (float)m_PreviewSize.y};

    Game *game = Physbuzz::Context::get<Game>();
    Physbuzz::Renderer &renderer = game->renderer.getRenderer();

    // TODO buttons
    for (auto &object : m_Scene.getObjects()) {
        PickableComponent &pickable = object.getComponent<PickableComponent>();

        // render to framebuffer
        renderer.target(&pickable.framebuffer);
        renderer.clear(bgColor);
        renderer.render(object.getComponent<Physbuzz::RenderComponent>());

        // imgui fuckery
        ImGui::Image((void *)(intptr_t)pickable.framebuffer.getColor(), size);
    }

    // release target
    renderer.target(nullptr);

    ImGui::End();
}
