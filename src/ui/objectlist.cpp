#include "objectlist.hpp"

#include "../objects/circle.hpp"
#include "../objects/cuboid.hpp"
#include "../objects/quad.hpp"
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <physbuzz/render/components/lights.hpp>
#include <physbuzz/render/defines.hpp>
#include <unordered_set>

constexpr float MAX_VALUE = 1000.0f;
constexpr float MIN_VALUE = -1000.0f;

ObjectList::ObjectList(Physbuzz::Scene *scene)
    : IUserInterface(scene) {}

void ObjectList::draw() {
    const ImGuiViewport *Viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(Viewport->WorkPos.x, Viewport->WorkPos.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(128, 256), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags windowFlags = 0;
    if (!ImGui::Begin("ObjectList", &show, windowFlags)) {
        ImGui::End();
        return;
    }

    const std::unordered_set<Physbuzz::ObjectID> &objects = m_Scene->getObjects();

    ImGui::Text("Simulate Physics: %s", m_Scene->getSystem<Physbuzz::Dynamics>()->isRunning() ? "true" : "false");

    if (ImGui::Button("Toggle")) {
        m_Scene->getSystem<Physbuzz::Dynamics>()->toggle();
    }

    ImGui::Text("Spawned Objects: %zu", objects.size());

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Objects")) {

        int i = 0;
        for (const auto &object : objects) {
            ImGui::PushID(i++);
            bool rebuild = false;
            if (!m_Scene->containsComponent<IdentifiableComponent>(object)) {
                ImGui::PopID();
                continue;
            }

            const auto [identifier] = m_Scene->getComponent<IdentifiableComponent>(object);

            if (identifier.hidden) {
                ImGui::PopID();
                continue;
            }

            ImGui::SeparatorText(std::format("{}) {}", object, identifier.name).c_str());

            if (m_Scene->containsComponent<Physbuzz::RenderComponent>(object)) {
                ImGui::SeparatorText("Render");

                const auto [render] = m_Scene->getComponent<Physbuzz::RenderComponent>(object);

                Physbuzz::Transform::Info info = render.transform.getInfo();

                if (ImGui::DragFloat3("position", glm::value_ptr(info.position), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    render.transform.update(info);
                }

                if (ImGui::DragFloat3("scale", glm::value_ptr(info.scale), 0.1f, MIN_VALUE, MAX_VALUE)) {
                    render.transform.update(info);
                }

                glm::vec3 axis = glm::axis(info.orientation);
                float angle = glm::angle(info.orientation);

                if (ImGui::DragFloat3("rotAxis", glm::value_ptr(axis), 0.01f, 0.0f, 1.0f)) {
                    info.orientation = glm::angleAxis(angle, glm::normalize(axis));
                    render.transform.update(info);
                }

                if (ImGui::DragFloat("rotMag", &angle, glm::pi<float>() / 50.0f, 0.0f, 2 * glm::pi<float>())) {
                    info.orientation = glm::angleAxis(angle, glm::normalize(axis));
                    render.transform.update(info);
                }
            }

            if (m_Scene->containsComponent<Physbuzz::RigidBodyComponent>(object)) {
                ImGui::SeparatorText("RigidBody");

                const auto [physics] = m_Scene->getComponent<Physbuzz::RigidBodyComponent>(object);

                ImGui::DragFloat("mass", &physics.mass, 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat3("velocity", glm::value_ptr(physics.velocity), 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat3("acceleration", glm::value_ptr(physics.acceleration), 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat3("gravity", glm::value_ptr(physics.gravity.acceleration), 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat2("drag", &physics.drag.k1, 0.01f, -MAX_VALUE, MAX_VALUE);
            }

            if (m_Scene->containsComponent<QuadComponent>(object)) {
                ImGui::SeparatorText("Quad");

                const auto [quad] = m_Scene->getComponent<QuadComponent>(object);
                float wh[] = {quad.width, quad.height};

                if (ImGui::DragFloat2("quad", wh, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    quad.width = wh[0];
                    quad.height = wh[1];

                    rebuild = true;
                }
            }

            if (m_Scene->containsComponent<CuboidComponent>(object)) {
                ImGui::SeparatorText("Cube");

                const auto [cube] = m_Scene->getComponent<CuboidComponent>(object);
                float whl[] = {cube.width, cube.breadth, cube.height};

                if (ImGui::DragFloat3("cube", whl, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    cube.width = whl[0];
                    cube.breadth = whl[1];
                    cube.height = whl[2];

                    rebuild = true;
                }
            }

            if (m_Scene->containsComponent<RadialComponent>(object)) {
                ImGui::SeparatorText("Radial");

                const auto [radial] = m_Scene->getComponent<RadialComponent>(object);
                if (ImGui::DragFloat("radius", &radial.radius, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }
            }

            if (m_Scene->containsComponent<Physbuzz::PointLightComponent>(object)) {
                ImGui::SeparatorText("PointLight");

                const auto [pointLight] = m_Scene->getComponent<Physbuzz::PointLightComponent>(object);

                Physbuzz::PointLightComponent::Info info = pointLight.getInfo();

                if (ImGui::DragFloat3("position", glm::value_ptr(info.position), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    pointLight.update(info);
                }

                if (ImGui::DragFloat3("intensity", glm::value_ptr(info.intensity), 0.01f, MIN_VALUE, MAX_VALUE)) {
                    pointLight.update(info);
                }

                if (ImGui::DragFloat("depth", &info.depth, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    pointLight.update(info);
                }
            }

            if (m_Scene->containsComponent<Physbuzz::SpotLightComponent>(object)) {
                ImGui::SeparatorText("SpotLight");

                const auto [spotLight] = m_Scene->getComponent<Physbuzz::SpotLightComponent>(object);

                Physbuzz::SpotLightComponent::Info info = spotLight.getInfo();

                if (ImGui::DragFloat3("position", glm::value_ptr(info.position), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    spotLight.update(info);
                }

                if (ImGui::DragFloat3("direction", glm::value_ptr(info.direction), 0.01f, MIN_VALUE, MAX_VALUE)) {
                    spotLight.update(info);
                }

                if (ImGui::DragFloat3("intensity", glm::value_ptr(info.intensity), 0.01f, MIN_VALUE, MAX_VALUE)) {
                    spotLight.update(info);
                }

                if (ImGui::DragFloat("cutOff", &info.cutOff, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    spotLight.update(info);
                }

                if (ImGui::DragFloat("outerCutOff", &info.outerCutOff, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    spotLight.update(info);
                }
            }

            if (m_Scene->containsComponent<Physbuzz::DirectionalLightComponent>(object)) {
                ImGui::SeparatorText("DirectionalLight");

                const auto [directionalLight] = m_Scene->getComponent<Physbuzz::DirectionalLightComponent>(object);

                Physbuzz::DirectionalLightComponent::Info info = directionalLight.getInfo();

                if (ImGui::DragFloat3("direction", glm::value_ptr(info.direction), 0.01f, MIN_VALUE, MAX_VALUE)) {
                    directionalLight.update(info);
                }

                if (ImGui::DragFloat3("intensity", glm::value_ptr(info.intensity), 0.01f, MIN_VALUE, MAX_VALUE)) {
                    directionalLight.update(info);
                }

                if (ImGui::DragFloat("orthoSize", &info.orthoSize, 0.01f, MIN_VALUE, MAX_VALUE)) {
                    directionalLight.update(info);
                }

                if (ImGui::DragFloat("depth", &info.depth, 0.01f, MIN_VALUE, MAX_VALUE)) {
                    directionalLight.update(info);
                }
            }

            if (rebuild && m_Scene->containsComponent<RebuildableComponent>(object)) {
                const auto [rebuilder] = m_Scene->getComponent<RebuildableComponent>(object);
                rebuilder.rebuild(*m_Scene, object);
            }

            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    ImGui::End();
}
