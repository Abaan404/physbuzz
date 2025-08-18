#pragma once

#include "../io/file.hpp"
#include "../resources/defines.hpp"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class VertexDescription;
class Scene;

template <typename T>
concept ShaderPODType =
    std::same_as<T, float> ||
    std::same_as<T, glm::vec2> ||
    std::same_as<T, glm::vec3> ||
    std::same_as<T, glm::vec4> ||
    std::same_as<T, int> ||
    std::same_as<T, glm::ivec2> ||
    std::same_as<T, glm::ivec3> ||
    std::same_as<T, glm::ivec4> ||
    std::same_as<T, unsigned int> ||
    std::same_as<T, glm::uvec2> ||
    std::same_as<T, glm::uvec3> ||
    std::same_as<T, glm::uvec4> ||
    std::same_as<T, glm::mat2> ||
    std::same_as<T, glm::mat3> ||
    std::same_as<T, glm::mat4> ||
    std::same_as<T, glm::mat2x3> ||
    std::same_as<T, glm::mat3x2> ||
    std::same_as<T, glm::mat2x4> ||
    std::same_as<T, glm::mat4x2> ||
    std::same_as<T, glm::mat3x4> ||
    std::same_as<T, glm::mat4x3>;

class ShaderPipeline {
  public:
    using Stage = vk::ShaderStageFlagBits;
    using DynamicState = vk::DynamicState;
    using PrimitiveTopology = vk::PrimitiveTopology;

    struct Shader {
        std::string entrypoint;
    };

    struct Info {
        File::Info module;
        PrimitiveTopology topology = PrimitiveTopology::eTriangleList;

        VertexDescription *description;

        std::unordered_map<std::uint32_t, std::vector<std::byte>> specializations = {};
        std::unordered_map<Stage, Shader> shaders;

        std::vector<DynamicState> states = {};
    };

    ShaderPipeline(const Info &info);
    ~ShaderPipeline();

    bool build();
    bool destroy();

    bool reload();

    void bind(const vk::CommandBuffer &commandBuffer) const;

    template <ShaderPODType T>
    inline void setUniform(const std::string &, const T &) const {
        // setUniformInternal(glGetUniformLocation(m_Program, name.c_str()), data);
    }

    const Info &getInfo() const;

  private:
    vk::PipelineLayout m_Layout = nullptr;
    vk::Pipeline m_Pipeline = nullptr;

    bool m_FailedReload = false;
    bool m_RequestedReload = false;
    // std::function<void(const ResourceWatcherData &)> m_ReloadCallback;

    Info m_Info;

    template <ResourceType T>
    friend class ResourceRegistry;
};

template <>
struct IsResource<ShaderPipeline> : std::true_type {};

} // namespace Physbuzz
