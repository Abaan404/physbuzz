#include "objectlist.hpp"

#include "../../game.hpp"
#include "../../objects/circle.hpp"
#include "../../objects/line.hpp"
#include "../../objects/quad.hpp"
#include <format>
#include <glm/ext/scalar_constants.hpp>
#include <imgui.h>
#include <physbuzz/misc/context.hpp>

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

            if (game->scene.containsComponent<Physbuzz::TransformableComponent>(object)) {
                Physbuzz::TransformableComponent &transform = game->scene.getComponent<Physbuzz::TransformableComponent>(object);
                glm::vec3 norm = glm::axis(transform.orientation);

                float position[] = {transform.position.x, transform.position.y, transform.position.z};
                float scale[] = {transform.scale.x, transform.scale.y, transform.scale.z};
                float rotationAxis[] = {norm.x, norm.y, norm.z};
                float rotationAngle = glm::angle(transform.orientation);

                if (ImGui::DragFloat3("position", position, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    glm::vec3 tmp = glm::vec3(position[0], position[1], position[2]);
                    transform.position = tmp;

                    rebuild = true;
                }

                if (ImGui::DragFloat3("scale", scale, 0.1f, MIN_VALUE, MAX_VALUE)) {
                    glm::vec3 tmp = glm::vec3(scale[0], scale[1], scale[2]);
                    transform.scale = tmp;

                    rebuild = true;
                }

                if (ImGui::DragFloat3("rotAxis", rotationAxis, 0.01f, 0.0f, 1.0f)) {
                    glm::vec3 ret = glm::normalize(glm::vec3(rotationAxis[0], rotationAxis[1], rotationAxis[2]));
                    transform.orientation = glm::angleAxis(rotationAngle, glm::vec3(ret[0], ret[1], ret[2]));

                    rebuild = true;
                }

                if (ImGui::DragFloat("rotMag", &rotationAngle, glm::pi<float>() / 50.0f, 0, 2 * glm::pi<float>())) {
                    transform.orientation = glm::angleAxis(rotationAngle, glm::vec3(rotationAxis[0], rotationAxis[1], rotationAxis[2]));

                    rebuild = true;
                }
            }

            if (game->scene.containsComponent<Physbuzz::RigidBodyComponent>(object)) {
                Physbuzz::RigidBodyComponent &physics = game->scene.getComponent<Physbuzz::RigidBodyComponent>(object);
                float velocity[] = {physics.velocity.x, physics.velocity.y, physics.velocity.z};
                float acceleration[] = {physics.acceleration.x, physics.acceleration.y, physics.acceleration.z};
                float gravity[] = {physics.gravity.acceleration.x, physics.gravity.acceleration.y, physics.gravity.acceleration.z};
                float drag[] = {physics.drag.k1, physics.drag.k2};

                ImGui::DragFloat("mass", &physics.mass, 0.01f, -MAX_VALUE, MAX_VALUE);

                if (ImGui::DragFloat3("velocity", velocity, 0.01f, -MAX_VALUE, MAX_VALUE)) {
                    glm::vec3 tmp = glm::vec3(velocity[0], velocity[1], velocity[2]);
                    physics.velocity = tmp;
                }

                if (ImGui::DragFloat3("acceleration", acceleration, 0.01f, -MAX_VALUE, MAX_VALUE)) {
                    glm::vec3 tmp = glm::vec3(acceleration[0], acceleration[1], velocity[2]);
                    physics.acceleration = tmp;
                }

                if (ImGui::DragFloat3("gravity", gravity, 0.01f, -MAX_VALUE, MAX_VALUE)) {
                    glm::vec3 tmp = glm::vec3(gravity[0], gravity[1], gravity[2]);
                    physics.gravity.acceleration = tmp;
                }

                if (ImGui::DragFloat2("drag", drag, 0.01f, -MAX_VALUE, MAX_VALUE)) {
                    physics.drag.k1 = drag[0];
                    physics.drag.k2 = drag[1];
                }
            }

            if (game->scene.containsComponent<LineComponent>(object)) {
                LineComponent &line = game->scene.getComponent<LineComponent>(object);
                float lt[] = {line.length, line.thickness};

                if (ImGui::DragFloat2("line", lt, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    line.length = lt[0];
                    line.thickness = lt[1];

                    rebuild = true;
                }
            }

            if (game->scene.containsComponent<QuadComponent>(object)) {
                QuadComponent &quad = game->scene.getComponent<QuadComponent>(object);
                float wh[] = {quad.width, quad.height};

                if (ImGui::DragFloat2("quad", wh, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    quad.width = wh[0];
                    quad.height = wh[1];

                    rebuild = true;
                }
            }

            if (game->scene.containsComponent<CircleComponent>(object)) {
                CircleComponent &radius = game->scene.getComponent<CircleComponent>(object);
                if (ImGui::DragFloat("circle", &radius.radius, 1.0f, MIN_VALUE, MAX_VALUE)) {
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
