#pragma once

#if !defined(IMGUI_DISABLE)

#include "../../ecs/system.hpp"
#include "../../graphics/rendergraph.hpp"
#include "../../resources/registry.hpp"
#include <imgui.h>

namespace Physbuzz {

class Window;
class Texture;
class Sampler;

class ImGuiRenderer : public System<> {
  public:
    inline static RenderNodeID Output = "builtin/imgui";

    struct Info {
        std::shared_ptr<Window> window;
    };

    ImGuiRenderer(Scene *scene, const Info &info);

    bool build() override;
    bool destroy() override;

    void newFrame();

    const RenderGraph &getGraph() const;
    std::uint32_t getFrameInFlight() const;

    std::optional<ImTextureID> getTexture(const Resource<Texture> &texture, const Image::ViewInfo &viewInfo);
    std::optional<ImTextureID> getTexture(const Resource<Attachment> &attachment, const Image::ViewInfo &viewInfo);

  private:
    ImTextureID createTexture(const Resource<Texture> &texture, const vk::ImageView &view);
    ImTextureID createTexture(const Resource<Attachment> &attachment, const vk::ImageView &view);

    struct StoredTextureID {
        std::unordered_map<Image::ViewInfo, ImTextureID, Image::ViewInfoHash> textureIds;
        EventID rebuildId = -1;
    };

    struct StoredAttachmentID {
        std::unordered_map<Image::ViewInfo, std::array<ImTextureID, detail::MAX_FRAMES_IN_FLIGHT>, Image::ViewInfoHash> textureIds;
        EventID rebuildId = -1;
    };

    Info m_Info;

    Resource<Sampler> m_Sampler;

    std::uint32_t m_FrameInFlight = -1;

    std::unordered_map<Resource<Texture>, StoredTextureID> m_Textures;
    std::unordered_map<Resource<Attachment>, StoredAttachmentID> m_Attachments;

    RenderGraph m_Graph = {{
        .output = Output,
    }};
};

} // namespace Physbuzz

#endif
