#include "objectlist.hpp"

#include "../../game.hpp"
#include "../../objects/objects.hpp"
#include <physbuzz/renderer.hpp>

#include <format>
#include <imgui.h>
#include <vector>

constexpr float MAX_VALUE = 1000.0f;
constexpr float MIN_VALUE = 0.0f;

void ObjectList::draw(Physbuzz::Renderer &renderer) {
    const ImGuiViewport *Viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(Viewport->WorkPos.x, Viewport->WorkPos.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(128, 256), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags windowFlags = 0;
    if (!ImGui::Begin("ObjectList", &show, windowFlags)) {
        ImGui::End();
        return;
    }

    Game::scene.sortObjects([](const Physbuzz::Object &object1, const Physbuzz::Object &object2) {
        return object1.getId() < object2.getId();
    });

    std::vector<Physbuzz::Object> &objects = Game::scene.getObjects();

    ImGui::Text("Spawned Objects: %zu", objects.size());

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Objects")) {

        int i = 0;
        for (auto &object : objects) {
            ImGui::PushID(i++);
            bool rebuild = false;
            glm::ivec2 resolution = renderer.getResolution();

            if (object.hasComponent<IdentifiableComponent>()) {
                IdentifiableComponent &identifier = object.getComponent<IdentifiableComponent>();
                ImGui::SeparatorText(std::format("{}) {}", object.getId(), identifier.name).c_str());
            }

            if (object.hasComponent<TransformableComponent>()) {
                TransformableComponent &transform = object.getComponent<TransformableComponent>();
                float position[] = {transform.position.x, transform.position.y, transform.position.z};
                float scale[] = {transform.scale.x, transform.scale.y, transform.scale.z};

                if (ImGui::DragFloat3("position", position, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    glm::vec3 dpos = glm::vec3(position[0], position[1], position[2]);
                    transform.position = dpos;

                    rebuild = true;
                }

                if (ImGui::DragFloat3("scale", scale, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    glm::vec3 dscale = glm::vec3(scale[0], scale[1], scale[2]);
                    transform.scale = dscale;

                    rebuild = true;
                }
            }

            if (object.hasComponent<RigidBodyComponent>()) {
                RigidBodyComponent &physics = object.getComponent<RigidBodyComponent>();
                float velocity[] = {physics.velocity.x, physics.velocity.y, physics.velocity.z};
                float acceleration[] = {physics.acceleration.x, physics.acceleration.y, physics.acceleration.z};

                if (ImGui::DragFloat3("velocity", velocity, 0.01f, -MAX_VALUE, MAX_VALUE)) {
                    glm::vec3 dv = glm::vec3(velocity[0], velocity[1], velocity[2]);
                    physics.velocity = dv;
                }

                if (ImGui::DragFloat3("acceleration", acceleration, 0.01f, -MAX_VALUE, MAX_VALUE)) {
                    glm::vec3 da = glm::vec3(acceleration[0], acceleration[1], velocity[2]);
                    physics.acceleration = da;
                }
            }

            if (object.hasComponent<QuadComponent>()) {
                QuadComponent &quad = object.getComponent<QuadComponent>();
                float wh[] = {quad.width, quad.height};

                if (ImGui::DragFloat2("Quad", wh, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    TransformableComponent &transform = object.getComponent<TransformableComponent>();
                    quad.width = wh[0];
                    quad.height = wh[1];

                    rebuild = true;
                }
            }

            if (object.hasComponent<CircleComponent>()) {
                CircleComponent &radius = object.getComponent<CircleComponent>();
                if (ImGui::DragFloat("Circle", &radius.radius, 1.0f, MIN_VALUE, MAX_VALUE)) {
                    rebuild = true;
                }
            }

            if (rebuild && object.hasComponent<IdentifiableComponent>()) {

                if (object.hasComponent<Physbuzz::MeshComponent>()) {
                    object.getComponent<Physbuzz::MeshComponent>().destroy();
                }

                IdentifiableComponent &identifier = object.getComponent<IdentifiableComponent>();
                {
                    switch (identifier.type) {
                    case (ObjectType::Box): {
                        QuadComponent &quad = object.getComponent<QuadComponent>();
                        TransformableComponent &transform = object.getComponent<TransformableComponent>();

                        float width = quad.width;
                        float height = quad.height;
                        glm::vec3 position = transform.position;

                        object.eraseComponents();
                        buildBox(object, position, width, height);
                    } break;

                    case (ObjectType::Circle): {
                        glm::vec3 position = object.getComponent<TransformableComponent>().position;
                        float radius = object.getComponent<CircleComponent>().radius;

                        object.eraseComponents();
                        buildCircle(object, position, radius);
                    } break;

                    default:
                        break;
                    }
                }
            }

            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    ImGui::End();
}