#pragma once

#if !defined(IMGUI_DISABLE)

#include "../../ecs/system.hpp"
#include "../../graphics/rendergraph.hpp"
#include "../../resources/resource.hpp"
#include <imgui.h>

namespace Physbuzz {

class Window;
class Texture;
class Sampler;

namespace Builtin {

namespace RenderPipelineImGui {

inline Resource<Sampler> ResourceSampler = {"builtin/imgui/sampler"};

bool build();

} // namespace RenderPipelineImGui

} // namespace Builtin

class ImGuiRenderer : public System<> {
  public:
    constexpr static RenderNodeID Output = "builtin/imgui";

    struct Info {
        std::shared_ptr<Window> window;
    };

    ImGuiRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void newFrame();

    const RenderGraph &getGraph() const;

    ImTextureID getTexture(const Resource<Texture> &texture);
    ImTextureID getTexture(const Resource<Attachment> &attachment, std::uint32_t frameInFlight);

  private:
    Info m_Info;

    std::unordered_map<Resource<Texture>, std::tuple<ImTextureID, EventID>> m_Textures;
    std::unordered_map<Resource<Attachment>, std::tuple<std::array<ImTextureID, detail::MAX_FRAMES_IN_FLIGHT>, EventID>> m_Attachments;

    vk::DescriptorPool m_Pool = nullptr;

    RenderGraph m_Graph = {{
        .output = Output,
    }};
};

} // namespace Physbuzz

#endif
