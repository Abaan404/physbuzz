#include "rendergraph.hpp"

#include <imgui.h>
#include <physbuzz/compat/imgui/imgui_impl_physbuzz.hpp>
#include <physbuzz/graphics/renderer.hpp>
#include <physbuzz/render/deferred.hpp>
#include <physbuzz/render/forward.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/render/skybox.hpp>

static void drawImageWindow(std::string label, bool *show, ImTextureID id, const glm::uvec2 &resolution) {
    float aspect_ratio = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);

    ImGui::SetNextWindowSizeConstraints(
        ImVec2(427, 240),
        ImVec2(resolution.x, resolution.y),
        [](ImGuiSizeCallbackData *data) {
            // https://github.com/ocornut/imgui/pull/8028
            float aspect_ratio = *(float *)data->UserData;
            data->DesiredSize.y = data->DesiredSize.x / aspect_ratio;

            switch (ImGui::GetMouseCursor()) {
            case ImGuiMouseCursor_ResizeNWSE:
            case ImGuiMouseCursor_ResizeNESW:
                if (aspect_ratio > data->DesiredSize.x / data->DesiredSize.y) {
                    data->DesiredSize.x = aspect_ratio * data->DesiredSize.y;
                } else {
                    data->DesiredSize.y = data->DesiredSize.x / aspect_ratio;
                }
                break;

            case ImGuiMouseCursor_ResizeNS:
                data->DesiredSize.x = aspect_ratio * data->DesiredSize.y;
                break;
            case ImGuiMouseCursor_ResizeEW:
                data->DesiredSize.y = data->DesiredSize.x / aspect_ratio;
                break;
            }
        },
        (void *)&aspect_ratio);

    if (!ImGui::Begin(label.c_str(), show)) {
        ImGui::End();
        return;
    }

    ImGui::Image(id, ImGui::GetContentRegionAvail());
    ImGui::End();
}

RenderGraph::RenderGraph(Physbuzz::Scene *scene)
    : IUserInterface(scene) {}

void RenderGraph::draw() {
    const std::shared_ptr<Physbuzz::Renderer> renderer = m_Scene->getSystem<Physbuzz::Renderer>();

    ImGuiWindowFlags windowFlags = 0;
    if (!ImGui::Begin("RenderGraph", &show, windowFlags)) {
        ImGui::End();
        return;
    }

    const Physbuzz::RenderGraph &graph = renderer->getGraph();

    bool updateGraph = false;

    static bool enableShadows = false;
    updateGraph |= ImGui::Checkbox("Shadows", &enableShadows);

    const char *types[] = {"Deferred", "Forward", "Unknown"};
    static int currentType = 2;

    for (const auto &node : graph.getExecutableNodes()) {
        if (node == Physbuzz::DeferredRenderer::Output) {
            currentType = 0;
            break;
        }

        if (node == Physbuzz::ForwardRenderer::Output) {
            currentType = 1;
            break;
        }

        currentType = 2;
    }

    updateGraph |= ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types));

    ImGui::SeparatorText("Nodes");

    for (const auto &id : graph.getExecutableNodes()) {
        if (ImGui::TreeNode(id.c_str())) {
            const Physbuzz::RenderNode &node = graph.get(id);

            if (ImGui::TreeNode("buffers")) {
                for (const auto &[buffer, _] : node.description.buffers.input) {
                    ImGui::Text("input: %s", buffer.getIdentifier().c_str());
                }

                for (const auto &[buffer, _] : node.description.buffers.output) {
                    ImGui::Text("output: %s", buffer.getIdentifier().c_str());
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("attachments")) {
                for (const auto &[attachment, _] : node.description.attachments.input) {
                    ImGui::Text("input: %s", attachment.getIdentifier().c_str());
                }

                for (const auto &[attachment, _] : node.description.attachments.output) {
                    ImGui::Text("output: %s", attachment.getIdentifier().c_str());
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    ImGui::SeparatorText("Resources");

    static bool showWindow = false;
    static Physbuzz::Resource<Physbuzz::Attachment> selectedAttachment = {""};

    const Physbuzz::RenderGraph::Resources &resources = graph.getResources();
    if (ImGui::TreeNode("buffers")) {
        for (const auto &resource : resources.buffers) {
            ImGui::Text("%s", resource.getIdentifier().c_str());
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("attachments")) {
        for (const auto &resource : resources.attachments) {
            ImGui::PushID(resource.getIdentifier().c_str());

            ImGui::Text("%s", resource.getIdentifier().c_str());

            if (ImGui::Button("Show")) {
                selectedAttachment = resource;
                showWindow = true;
            }

            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    std::shared_ptr<Physbuzz::ImGuiRenderer> imguiImpl = m_Scene->getSystem<Physbuzz::ImGuiRenderer>();

    if (showWindow && Physbuzz::ResourceRegistry<Physbuzz::Attachment>::contains(selectedAttachment)) {
        drawImageWindow(
            "Attachment",
            &showWindow,
            imguiImpl->getTexture(selectedAttachment, renderer->getFrameInFlight()),
            {selectedAttachment->getSize(renderer->getFrameInFlight()).x, selectedAttachment->getSize(renderer->getFrameInFlight()).y});
    }

    if (updateGraph) {
        Physbuzz::RenderGraph graph = {{}};

        std::shared_ptr<Physbuzz::ShadowRenderer> shadow = m_Scene->getSystem<Physbuzz::ShadowRenderer>();
        std::shared_ptr<Physbuzz::DeferredRenderer> deferred = m_Scene->getSystem<Physbuzz::DeferredRenderer>();
        std::shared_ptr<Physbuzz::ForwardRenderer> forward = m_Scene->getSystem<Physbuzz::ForwardRenderer>();
        std::shared_ptr<Physbuzz::ImGuiRenderer> imgui = m_Scene->getSystem<Physbuzz::ImGuiRenderer>();
        std::shared_ptr<Physbuzz::SkyboxRenderer> skybox = m_Scene->getSystem<Physbuzz::SkyboxRenderer>();

        if (enableShadows) {
            graph.merge(shadow->getGraph());
        }

        switch (currentType) {
        case 0: // Deferred
            deferred->specialize({
                .enableShadows = enableShadows,
            });

            graph.merge(deferred->getGraph());
            graph.merge(skybox->getGraph());
            graph.merge(imgui->getGraph());
            break;

        case 1: // Forward
        default:
            forward->specialize({
                .enableShadows = enableShadows,
            });

            graph.merge(forward->getGraph());
            graph.merge(skybox->getGraph());
            graph.merge(imgui->getGraph());
            break;
        }

        if (graph.compile()) {
            renderer->setGraph(graph);
        }
    }

    ImGui::End();
}
