#include "objectpicker.hpp"

#include "../objects/builder.hpp"
#include "../objects/circle.hpp"
#include "../objects/quad.hpp"
#include "../objects/skybox.hpp"
#include <glm/glm.hpp>
#include <imgui.h>
#include <physbuzz/render/camera.hpp>
#include <physbuzz/render/renderer.hpp>

struct PickableComponent {
    bool selected = false;
    // Physbuzz::Framebuffer framebuffer;
};

ObjectPicker::ObjectPicker(Physbuzz::Scene *scene)
    : IUserInterface(scene) {
    Quad quad = {
        .body = {},
        .quad = {
            .width = m_PreviewSize.x,
            .height = m_PreviewSize.y,
        },
        .transform = {
            .position = {m_PreviewSize.x / 2.0f, m_PreviewSize.y / 2.0f, 0.0f},
        },
        .resources = {},
    };

    Circle circle = {
        .body = {},
        .circle = {
            .radius = glm::min(m_PreviewSize.x, m_PreviewSize.y) / 2.0f,
        },
        .transform = {
            .position = {m_PreviewSize.x / 2.0f, m_PreviewSize.y / 2.0f, 0.0f},
        },
        .resources = {},
    };

    Skybox skybox;

    // m_PickerScene.createSystem<Physbuzz::Renderer>(Physbuzz::Renderer::Info{
    //     // .passthrough = false,
    //     // .resolution = {m_PreviewSize.x, m_PreviewSize.y},
    // });

    ObjectBuilder::create(m_PickerScene, circle);
    ObjectBuilder::create(m_PickerScene, quad);

    Physbuzz::CameraComponent camera = {{
        .resolution = {m_PreviewSize.x, m_PreviewSize.y},
        .projection = Physbuzz::CameraComponent::Projection::Orthographic,
        .orthographic = {
            .left = 0.0f,
            .right = static_cast<float>(m_PreviewSize.x),
            .bottom = static_cast<float>(m_PreviewSize.y),
            .top = 0.0f,
        },
        .perspective = {},
        .depth = {
            .near = 1.0f,
            .far = 1000.0f,
        },
        .view = {
            .position = {0.0f, 0.0f, -100.0f},
        },
    }};

    Physbuzz::ObjectID object = m_PickerScene.createObject();
    m_PickerScene.setComponent(object, camera);

    for (const auto &object : m_PickerScene.getObjects()) {
        PickableComponent pickable = {
            .selected = false,
            // .framebuffer = {
            //     {
            //         .resolution = {m_PreviewSize.x, m_PreviewSize.y},
            //         .clear = {},
            //         .colors = {
            //             {
            //                 .storage = Physbuzz::Framebuffer::Storage::Texture2D,
            //                 .isDrawn = true,
            //             },
            //         },
            //         .depth = {},
            //     },
            // },
        };

        // pickable.framebuffer.build();
        m_PickerScene.setComponent(object, pickable);
    }
}

ObjectPicker::~ObjectPicker() {
    for (const auto &[_, pickable] : m_PickerScene.getComponents<PickableComponent>()) {
        // pickable.framebuffer.destroy();
    }

    m_PickerScene.clear();
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

    // for (const auto &[camera] : m_Scene.getComponents<Physbuzz::CameraComponent>()) {
    //     Physbuzz::Resource<Physbuzz::UniformBuffer<UniformCamera>>("camera")->update({
    //         .position = camera.getInfo().view.position,
    //         ._padding0 = 0.0f,
    //         .view = camera.getView(),
    //         .projection = camera.getProjection(),
    //     });
    // }

    // TODO this doesnt work
    // for (const auto &object : m_Scene.getObjects()) {
    //     const auto [pickable] = m_Scene.getComponent<PickableComponent>(object);
    //
    //     // render to framebuffer
    //     // m_Scene.getSystem<Physbuzz::Renderer>()->target(&pickable.framebuffer);
    //     // m_Scene.tickSystem<Physbuzz::Renderer>();
    //
    //     const auto [_, image] = pickable.framebuffer.getInfo().colors[0];
    //     ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(image)), m_PreviewSize);
    // }

    ImGui::End();
}
