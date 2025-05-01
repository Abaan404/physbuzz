#include "objectlist.hpp"

#include "../../game.hpp"
#include "../../objects/circle.hpp"
#include "../../objects/line.hpp"
#include "../../objects/quad.hpp"
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/lighting.hpp>

constexpr float MAX_VALUE = 1000.0f;
constexpr float MIN_VALUE = -1000.0f;

void ObjectList::draw() {
    const ImGuiViewport *Viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(Viewport->WorkPos.x, Viewport->WorkPos.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(128, 256), ImGuiCond_FirstUseEver);

    Game *game = Physbuzz::Context::get<Game>();

    ImGuiWindowFlags windowFlags = 0;
    if (!ImGui::Begin("ObjectList", &show, windowFlags)) {
        ImGui::End();
        return;
    }

    const std::set<Physbuzz::ObjectID> &objects = game->scene.getObjects();

    ImGui::Text("Simulate Physics: %s", game->scene.getSystem<Physbuzz::Dynamics>()->isRunning() ? "true" : "false");

    if (ImGui::Button("Toggle")) {
        game->scene.getSystem<Physbuzz::Dynamics>()->toggle();
    }

    ImGui::Text("Spawned Objects: %zu", objects.size());

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Objects")) {

        int i = 0;
        for (const auto &object : objects) {
            ImGui::PushID(i++);
            bool rebuild = false;

            if (game->scene.containsComponent<IdentifiableComponent>(object)) {
                IdentifiableComponent &identifier = game->scene.getComponent<IdentifiableComponent>(object);

                if (identifier.hidden) {
                    ImGui::PopID();
                    continue;
                }

                ImGui::SeparatorText(std::format("{}) {}", object, identifier.name).c_str());
            }

            if (game->scene.containsComponent<Physbuzz::TransformComponent>(object)) {
                ImGui::SeparatorText("Transfrom");

                Physbuzz::TransformComponent &transform = game->scene.getComponent<Physbuzz::TransformComponent>(object);

                if (ImGui::DragFloat3("position", glm::value_ptr(transform.position), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    transform.update();
                }

                if (ImGui::DragFloat3("scale", glm::value_ptr(transform.scale), 0.1f, MIN_VALUE, MAX_VALUE)) {
                    transform.update();
                }

                glm::vec3 axis = glm::axis(transform.orientation);
                float angle = glm::angle(transform.orientation);

                if (ImGui::DragFloat3("rotAxis", glm::value_ptr(axis), 0.01f, 0.0f, 1.0f)) {
                    transform.orientation = glm::angleAxis(angle, glm::normalize(axis));
                    transform.update();
                }

                if (ImGui::DragFloat("rotMag", &angle, glm::pi<float>() / 50.0f, 0.0f, 2 * glm::pi<float>())) {
                    transform.orientation = glm::angleAxis(angle, glm::normalize(axis));
                    transform.update();
                }
            }

            if (game->scene.containsComponent<Physbuzz::RigidBodyComponent>(object)) {
                ImGui::SeparatorText("RigidBody");

                Physbuzz::RigidBodyComponent &physics = game->scene.getComponent<Physbuzz::RigidBodyComponent>(object);

                ImGui::DragFloat("mass", &physics.mass, 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat3("velocity", glm::value_ptr(physics.velocity), 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat3("acceleration", glm::value_ptr(physics.acceleration), 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat3("gravity", glm::value_ptr(physics.gravity.acceleration), 0.01f, -MAX_VALUE, MAX_VALUE);
                ImGui::DragFloat2("drag", &physics.drag.k1, 0.01f, -MAX_VALUE, MAX_VALUE);
            }

            if (game->scene.containsComponent<LineComponent>(object)) {
                ImGui::SeparatorText("Line");

                LineComponent &line = game->scene.getComponent<LineComponent>(object);
                float lt[] = {line.length, line.thickness};

                if (ImGui::DragFloat2("line", lt, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    line.length = lt[0];
                    line.thickness = lt[1];

                    rebuild = true;
                }
            }

            if (game->scene.containsComponent<QuadComponent>(object)) {
                ImGui::SeparatorText("Quad");

                QuadComponent &quad = game->scene.getComponent<QuadComponent>(object);
                float wh[] = {quad.width, quad.height};

                if (ImGui::DragFloat2("quad", wh, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    quad.width = wh[0];
                    quad.height = wh[1];

                    rebuild = true;
                }
            }

            if (game->scene.containsComponent<CircleComponent>(object)) {
                ImGui::SeparatorText("Circle");

                CircleComponent &radius = game->scene.getComponent<CircleComponent>(object);
                if (ImGui::DragFloat("circle", &radius.radius, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }
            }

            if (game->scene.containsComponent<Physbuzz::PointLightComponent>(object)) {
                ImGui::SeparatorText("PointLight");

                Physbuzz::PointLightComponent &pointLight = game->scene.getComponent<Physbuzz::PointLightComponent>(object);
                if (ImGui::DragFloat3("position", glm::value_ptr(pointLight.position), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("ambient", glm::value_ptr(pointLight.ambient), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("diffuse", glm::value_ptr(pointLight.diffuse), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("specular", glm::value_ptr(pointLight.specular), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat("constant", &pointLight.constant, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat("linear", &pointLight.linear, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat("quadratic", &pointLight.quadratic, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }
            }

            if (game->scene.containsComponent<Physbuzz::SpotLightComponent>(object)) {
                ImGui::SeparatorText("SpotLight");

                Physbuzz::SpotLightComponent &spotLight = game->scene.getComponent<Physbuzz::SpotLightComponent>(object);
                if (ImGui::DragFloat3("position", glm::value_ptr(spotLight.position), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("direction", glm::value_ptr(spotLight.direction), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("ambient", glm::value_ptr(spotLight.ambient), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("diffuse", glm::value_ptr(spotLight.diffuse), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("specular", glm::value_ptr(spotLight.specular), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat("constant", &spotLight.constant, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat("linear", &spotLight.linear, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat("quadratic", &spotLight.quadratic, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat("cutOff", &spotLight.cutOff, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat("outerCutOff", &spotLight.outerCutOff, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }
            }

            if (game->scene.containsComponent<Physbuzz::DirectionalLightComponent>(object)) {
                ImGui::SeparatorText("DirectionalLight");

                Physbuzz::DirectionalLightComponent &directionalLight = game->scene.getComponent<Physbuzz::DirectionalLightComponent>(object);
                if (ImGui::DragFloat3("direction", glm::value_ptr(directionalLight.direction), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("ambient", glm::value_ptr(directionalLight.ambient), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("diffuse", glm::value_ptr(directionalLight.diffuse), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }

                if (ImGui::DragFloat3("specular", glm::value_ptr(directionalLight.specular), 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }
            }

            if (rebuild && game->scene.containsComponent<RebuildableComponent>(object)) {
                game->scene.getComponent<RebuildableComponent>(object).rebuild(game->builder, object);
            }

            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    ImGui::End();
}
