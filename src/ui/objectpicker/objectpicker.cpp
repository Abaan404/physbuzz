#include "objectpicker.hpp"

#include "../../objects/builder.hpp"
#include "../../objects/circle.hpp"
#include "../../objects/quad.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/renderer.hpp>

ObjectPicker::ObjectPicker() {
    Quad quad = {
        .body = {},
        .quad = {
            .width = m_PreviewSize.x,
            .height = m_PreviewSize.y,
        },
        .transform = {
            .position = {m_PreviewSize.x / 2.0f, m_PreviewSize.y / 2.0f, 0.0f},
        },
        .resources = {
            .renderpasses = {
                {"ui/ortho"},
            },
        },
    };

    Circle circle = {
        .body = {},
        .circle = {
            .radius = glm::min(m_PreviewSize.x, m_PreviewSize.y) / 2.0f,
        },
        .transform = {
            .position = {m_PreviewSize.x / 2.0f, m_PreviewSize.y / 2.0f, 0.0f},
        },
        .resources = {
            .renderpasses = {
                {"ui/ortho"},
            },
        },
    };

    m_Scene.createSystem<Physbuzz::Renderer>(Physbuzz::RendererInfo{
        .framebuffer = {
            .resolution = {m_PreviewSize.x, m_PreviewSize.y},
            .colorClear = {0.0f, 0.0f, 0.0f, 0.0f},
        },
    });

    ObjectBuilder builder = ObjectBuilder(&m_Scene);
    builder.create(circle);
    builder.create(quad);

    for (const auto &object : m_Scene.getObjects()) {
        PickableComponent pickable = {
            .selected = false,
            .framebuffer = {{
                .resolution = {m_PreviewSize.x, m_PreviewSize.y},
                .colorClear = {0.0f, 0.0f, 0.0f, 0.0f},
            }},
        };

        pickable.framebuffer.build();
        m_Scene.setComponent(object, pickable);
    }

    // set orthographic projection for preview
    m_Camera.setOrthographic2D({m_PreviewSize.x, m_PreviewSize.y});
}

ObjectPicker::~ObjectPicker() {
    for (const auto &object : m_Scene.getObjects()) {
        if (m_Scene.containsComponent<PickableComponent>(object)) {
            const auto &[pickable] = m_Scene.getComponent<PickableComponent>(object);
            pickable.framebuffer.destroy();
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

    // TODO buttons
    for (const auto &object : m_Scene.getObjects()) {
        const auto &[pickable] = m_Scene.getComponent<PickableComponent>(object);

        // render to framebuffer
        m_Scene.getSystem<Physbuzz::Renderer>()->target(&pickable.framebuffer);
        m_Scene.tickSystem<Physbuzz::Renderer>();

        ImGui::Image((void *)(std::intptr_t)pickable.framebuffer.getColor(), m_PreviewSize);
    }

    ImGui::End();
}
