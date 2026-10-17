#include "rendergraph.hpp"

#include <imgui.h>
#include <physbuzz/compat/imgui/imgui_impl_physbuzz.hpp>
#include <physbuzz/graphics/renderer.hpp>
#include <physbuzz/render/deferred.hpp>
#include <physbuzz/render/forward.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/render/skybox.hpp>

RenderGraph::RenderGraph(Physbuzz::Scene *scene)
    : IUserInterface(scene) {}

void RenderGraph::draw() {
    Physbuzz::Renderer *renderer = m_Scene->getSystem<Physbuzz::Renderer>();

    ImGuiWindowFlags windowFlags = 0;
    if (!ImGui::Begin("RenderGraph", &show, windowFlags)) {
        ImGui::End();
        return;
    }

    const Physbuzz::RenderGraph &graph = renderer->getGraph();

    bool updateGraph = false;

    updateGraph |= ImGui::Checkbox("Shadows", &m_EnableShadows);

    const std::array nodes = {
        Physbuzz::DeferredRenderer::Output,
        Physbuzz::ForwardRenderer::Output,
    };

    for (const auto &renderer : nodes) {
        if (graph.contains(renderer)) {
            m_SelectedRenderer = renderer;
            break;
        }
    }

    if (ImGui::BeginCombo("Type", m_SelectedRenderer.data())) {
        for (const auto &node : nodes) {
            bool isSelected = graph.contains(node);

            if (ImGui::Selectable(node.data(), isSelected)) {
                updateGraph |= true;
                m_SelectedRenderer = node;
                break;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::SeparatorText("Nodes");

    for (const auto &id : graph.getExecutableNodes()) {
        if (ImGui::TreeNode(id.data())) {
            const Physbuzz::RenderNode &node = graph.get(id);

            if (ImGui::TreeNode("buffers")) {
                for (const auto &[buffer, _] : node.description.buffers.input) {
                    ImGui::Text("input: %s", buffer.getName().data());
                }

                for (const auto &[buffer, _] : node.description.buffers.output) {
                    ImGui::Text("output: %s", buffer.getName().data());
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("attachments")) {
                for (const auto &[attachment, _] : node.description.attachments.input) {
                    ImGui::Text("input: %s", attachment.getName().data());
                }

                for (const auto &[attachment, _] : node.description.attachments.output) {
                    ImGui::Text("output: %s", attachment.getName().data());
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    ImGui::SeparatorText("Resources");

    const Physbuzz::RenderGraph::Resources &resources = graph.getResources();
    if (ImGui::TreeNode("buffers")) {
        for (const auto &resource : resources.buffers) {
            ImGui::Text("%s", resource.getName().data());
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("attachments")) {
        for (const auto &resource : resources.attachments) {
            ImGui::PushID(resource.getName().data());

            ImGui::Text("%s", resource.getName().data());

            if (ImGui::Button("Show")) {
                m_AttachmentWindows.insert({resource, Image{resource, m_Scene}});
            }

            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    std::vector<Physbuzz::ResourceID> pendingRemoval;

    for (auto &[resource, image] : m_AttachmentWindows) {
        if (image.show) {
            image.draw();
        } else {
            pendingRemoval.emplace_back(resource);
        }
    }

    for (const auto &resource : pendingRemoval) {
        m_AttachmentWindows.erase(resource);
    }

    if (updateGraph) {
        Physbuzz::RenderGraph graph = {{}};

        Physbuzz::ShadowRenderer *shadow = m_Scene->getSystem<Physbuzz::ShadowRenderer>();
        Physbuzz::DeferredRenderer *deferred = m_Scene->getSystem<Physbuzz::DeferredRenderer>();
        Physbuzz::ForwardRenderer *forward = m_Scene->getSystem<Physbuzz::ForwardRenderer>();
        Physbuzz::ImGuiRenderer *imgui = m_Scene->getSystem<Physbuzz::ImGuiRenderer>();
        Physbuzz::SkyboxRenderer *skybox = m_Scene->getSystem<Physbuzz::SkyboxRenderer>();

        if (m_EnableShadows) {
            graph.merge(shadow->getGraph());
        }

        if (m_SelectedRenderer == Physbuzz::DeferredRenderer::Output) {
            deferred->specialize({
                .enableShadows = m_EnableShadows,
            });

            graph.merge(deferred->getGraph());
            graph.merge(skybox->getGraph());
            graph.merge(imgui->getGraph());

        } else if (m_SelectedRenderer == Physbuzz::ForwardRenderer::Output) {
            forward->specialize({
                .enableShadows = m_EnableShadows,
            });

            graph.merge(forward->getGraph());
            graph.merge(skybox->getGraph());
            graph.merge(imgui->getGraph());
        }

        if (graph.compile()) {
            renderer->setGraph(graph);
        }
    }

    ImGui::End();
}
