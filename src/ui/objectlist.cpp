#include "objectlist.hpp"

#include "../objects/common.hpp"
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <physbuzz/graphics/renderer.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/components/lights.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/render/shadow.hpp>
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

            if (m_Scene->containsComponent<Physbuzz::TransformComponent>(object)) {
                ImGui::SeparatorText("Transform");

                const auto [transform] = m_Scene->getComponent<Physbuzz::TransformComponent>(object);

                Physbuzz::Transform::Info info = transform.transform.getInfo();
                bool update = false;

                if (ImGui::DragFloat3("position", glm::value_ptr(info.position), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    update = true;
                }

                if (ImGui::DragFloat3("scale", glm::value_ptr(info.scale), 0.1f, MIN_VALUE, MAX_VALUE)) {
                    update = true;
                }

                glm::vec3 axis = glm::axis(info.orientation);
                float angle = glm::angle(info.orientation);

                if (ImGui::DragFloat3("rotAxis", glm::value_ptr(axis), 0.01f, 0.0f, 1.0f)) {
                    info.orientation = glm::angleAxis(angle, glm::normalize(axis));
                    update = true;
                }

                if (ImGui::DragFloat("rotMag", &angle, glm::pi<float>() / 50.0f, 0.0f, 2 * glm::pi<float>())) {
                    info.orientation = glm::angleAxis(angle, glm::normalize(axis));
                    update = true;
                }

                if (update) {
                    transform.transform.update(info);

                    if (m_Scene->containsComponent<Physbuzz::RigidBodyComponent>(object)) {
                        const auto [body] = m_Scene->getComponent<Physbuzz::RigidBodyComponent>(object);
                        body.applyTransform(info);
                    }
                }
            }

            if (m_Scene->containsComponent<Physbuzz::RenderComponent>(object)) {
                ImGui::SeparatorText("Render");

                const auto [render] = m_Scene->getComponent<Physbuzz::RenderComponent>(object);

                for (auto &material : render.model.getInfo().materials) {
                    Physbuzz::Renderer *renderer = m_Scene->getSystem<Physbuzz::Renderer>();

                    if (ImGui::DragFloat3("albedoFactor", glm::value_ptr(material->albedoFactor), 0.001f, 0.0f, 1.0f)) {
                        renderer->updateMaterial(material);
                    }

                    if (ImGui::DragFloat("metallicFactor", &material->metallicFactor, 0.001f, 0.0f, 1.0f)) {
                        renderer->updateMaterial(material);
                    }

                    if (ImGui::DragFloat("roughnessFactor", &material->roughnessFactor, 0.001f, 0.0f, 1.0f)) {
                        renderer->updateMaterial(material);
                    }
                }
            }

            if (m_Scene->containsComponent<Physbuzz::RigidBodyComponent>(object)) {
                ImGui::SeparatorText("RigidBody");

                const auto [body] = m_Scene->getComponent<Physbuzz::RigidBodyComponent>(object);

                Physbuzz::RigidBodyComponent::Info info = body.getInfo();

                ImGui::DragFloat("mass", &info.mass, 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat3("velocity", glm::value_ptr(info.velocity), 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat3("acceleration", glm::value_ptr(info.acceleration), 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat2("drag", &info.drag.k1, 0.01f, -MAX_VALUE, MAX_VALUE);

                body.update(info);
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

            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    ImGui::End();
}
