#include "objectpicker.hpp"

#include "../../game.hpp"
#include "../../objects/objects.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <physbuzz/scene.hpp>

ObjectPicker::ObjectPicker() {
    Physbuzz::ObjectID id1 = m_Scene.createObject();
    Physbuzz::ObjectID id2 = m_Scene.createObject();

    buildBox(m_Scene.getObject(id1), glm::vec3(m_PreviewSize.x >> 1, m_PreviewSize.y >> 1, 0.0f), m_PreviewSize.x, m_PreviewSize.y);
    buildCircle(m_Scene.getObject(id2), glm::vec3(m_PreviewSize.x >> 1, m_PreviewSize.y >> 1, 0.0f), glm::min(m_PreviewSize.x, m_PreviewSize.y) >> 1);

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

void ObjectPicker::draw(Physbuzz::Renderer &renderer) {
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

    // TODO buttons
    for (auto &object : m_Scene.getObjects()) {
        PickableComponent &pickable = object.getComponent<PickableComponent>();
        Physbuzz::MeshComponent &mesh = object.getComponent<Physbuzz::MeshComponent>();

        // render to framebuffer
        renderer.target(&pickable.framebuffer);
        renderer.clear(bgColor);
        renderer.render(mesh);

        // imgui fuckery
        ImGui::Image((void *)(intptr_t)pickable.framebuffer.getColor(), size);
    }

    // release target
    renderer.target(nullptr);

    ImGui::End();
}
