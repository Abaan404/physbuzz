#pragma once

#include <physbuzz/ecs/system.hpp>
#include <physbuzz/graphics/pipeline.hpp>
#include <physbuzz/render/batch.hpp>
#include <physbuzz/render/resources.hpp>
#include <physbuzz/resources/registry.hpp>
#include <physbuzz/window/window.hpp>

class CollisionHighlight : public Physbuzz::System<> {
  public:
    inline static Physbuzz::RenderNodeID Output = "builtin/collision_highlight";

    struct Info {
        std::shared_ptr<Physbuzz::Window> window;

        const std::vector<Physbuzz::ObjectID> &objects;
        const Physbuzz::RenderBatch &batch;
    };

    struct Resources {
        struct HighlightedObject {
            vk::Bool32 highlighted;
        };

        Physbuzz::Resource<Physbuzz::DescriptorLayout> layoutFrame;
        Physbuzz::Resource<Physbuzz::DynamicBuffer> highlightBuffer;
        Physbuzz::Resource<Physbuzz::GraphicsPipeline> pipeline;
    };

    CollisionHighlight(Physbuzz::Scene *scene, const Info &info);

    bool build() override;
    bool destroy() override;

    const Physbuzz::RenderGraph &getGraph();
    const Resources &getResources() const;

  private:
    Info m_Info;
    Resources m_Resources;

    std::vector<Resources::HighlightedObject> m_Highlighted;

    Physbuzz::RenderGraph m_Graph = {{
        .output = Output,
    }};

    struct {
        Physbuzz::EventID resize = -1;
    } m_Events = {};

    bool buildResources();
};
