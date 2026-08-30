#pragma once

#include "physbuzz/window/window.hpp"
#include <physbuzz/ecs/system.hpp>
#include <physbuzz/render/batch.hpp>

namespace PipelineCollisionHighlight {

struct HighlightedObject {
    vk::Bool32 highlighted;
};

inline Physbuzz::Resource<Physbuzz::DescriptorLayout> ResourceLayoutFrame = {"collision_highlight/frame"};
inline Physbuzz::Resource<Physbuzz::DynamicBuffer> ResourceHighlightBuffer = {"collision_highlight/highlight"};

inline Physbuzz::Resource<Physbuzz::GraphicsPipeline> Resource = {"collision_highlight"};

bool build();

} // namespace PipelineCollisionHighlight

class CollisionHighlight : public Physbuzz::System<> {
  public:
    inline static Physbuzz::RenderNodeID Output = "builtin/collision_highlight";

    struct Info {
        Physbuzz::ObjectID camera;
        std::shared_ptr<Physbuzz::Window> window;

        const Physbuzz::BatchGenerator &batch;
    };

    CollisionHighlight(Physbuzz::Scene *scene, const Info &info);

    bool build() override;
    bool destroy() override;

    const Physbuzz::RenderGraph &getGraph();

  private:
    Info m_Info;

    std::vector<PipelineCollisionHighlight::HighlightedObject> m_Highlighted;

    Physbuzz::RenderGraph m_Graph = {{
        .output = Output,
    }};

    struct {
        Physbuzz::EventID resize = -1;
    } m_Events = {};
};
