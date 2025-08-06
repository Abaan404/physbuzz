#pragma once

#include "../debug/logging.hpp"
#include "../ecs/defines.hpp"
#include "../io/file.hpp"
#include "../resources/defines.hpp"
#include <glm/glm.hpp>
#include <set>
#include <string>

namespace Physbuzz {

class Scene;

template <typename T>
concept UniformType =
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

class Shader {
  public:
    enum class Type {
        Vertex,
        TessControl,
        TessEvaluation,
        Geometry,
        Fragment,
        Compute,
        Unknown,
    };

    struct Info {
        File::Info file;
    };

    Shader(const Info &info, const Type &type);
    ~Shader();

    bool build();
    bool destroy();

    bool compile();

    void attach(std::uint32_t program) const;
    void detach(std::uint32_t program) const;

  private:
    const std::string preprocess(const File &file);
    bool preprocessInclude(const File &file, std::string &output, std::size_t position);

    std::uint32_t m_Shader = 0;
    Type m_Type = Type::Unknown;

    std::set<std::filesystem::path> m_Paths;

    Info m_Info;

    friend class ShaderPipeline;
};

class ShaderPipeline {
  public:
    struct Info {
        Shader::Info vertex;
        Shader::Info tessControl;
        Shader::Info tessEvaluation;
        Shader::Info geometry;
        Shader::Info fragment;
        Shader::Info compute;

        void (*draw)(const ShaderPipeline *, Scene &, ObjectID) = [](const ShaderPipeline *, Scene &, ObjectID id) {
            Logger::WARNING("[ShaderPipeline] Uninitialized draw calls for object '{}'", id);
        };
    };

    ShaderPipeline(const Info &info);
    ~ShaderPipeline();

    bool build();
    bool destroy();

    bool reload();
    void draw(Scene &scene, ObjectID object) const;

    void bind() const;
    void unbind() const;

    template <UniformType T>
    inline void setUniform(const std::string &, const T &) const {
        // setUniformInternal(glGetUniformLocation(m_Program, name.c_str()), data);
    }

    const Info &getInfo() const;

  private:
    std::uint32_t m_Program = 0;

    bool m_FailedReload = false;
    bool m_RequestedReload = false;
    std::function<void(const ResourceWatcherData &)> m_ReloadCallback;

    Info m_Info;

    template <ResourceType T>
    friend class ResourceRegistry;
};

template <>
struct IsResource<ShaderPipeline> : std::true_type {};

} // namespace Physbuzz
