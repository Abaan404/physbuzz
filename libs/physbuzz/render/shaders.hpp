#pragma once

#include "../debug/logging.hpp"
#include "../ecs/defines.hpp"
#include "../io/file.hpp"
#include "../resources/defines.hpp"
#include <format>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <set>
#include <string>

namespace Physbuzz {

class ShaderPipeline;
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
        Vertex = GL_VERTEX_SHADER,
        TessControl = GL_TESS_CONTROL_SHADER,
        TessEvaluation = GL_TESS_EVALUATION_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Compute = GL_COMPUTE_SHADER,
        Unknown = GL_INVALID_ENUM,
    };

    struct Info {
        File::Info file;
    };

    Shader(const Info &info, const Type &type);
    ~Shader();

    bool build();
    bool destroy();

    bool compile();

    bool attach(GLuint program) const;
    bool detach(GLuint program) const;

    const GLuint &getShader() const;
    const Type &getType() const;
    const std::set<std::filesystem::path> &getPaths() const;

  private:
    void preprocess(File &file);
    bool preprocessInclude(File &file, std::size_t position);

    GLuint m_Shader = 0;
    Type m_Type = Type::Unknown;

    std::set<std::filesystem::path> m_Paths;

    Info m_Info;
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

        void (*draw)(const ShaderPipeline *resource, Scene &, ObjectID) = [](const ShaderPipeline *, Scene &, ObjectID id) {
            Logger::WARNING(std::format("[ShaderPipelineResource] Uninitialized draw calls for object '{}'", id));
        };
    };

    ShaderPipeline(const Info &info);
    ~ShaderPipeline();

    bool build();
    bool destroy();

    bool reload();
    void draw(Scene &scene, ObjectID object) const;

    bool bind() const;
    bool unbind() const;

    template <UniformType T>
    inline void setUniform(const std::string &name, const T &data) const {
        setUniformInternal(glGetUniformLocation(m_Program, name.c_str()), data);
    }

    const GLuint &getProgram() const;

  private:
    void setUniformInternal(const GLint location, const float &data) const;
    void setUniformInternal(const GLint location, const glm::vec2 &data) const;
    void setUniformInternal(const GLint location, const glm::vec3 &data) const;
    void setUniformInternal(const GLint location, const glm::vec4 &data) const;

    void setUniformInternal(const GLint location, const int &data) const;
    void setUniformInternal(const GLint location, const glm::ivec2 &data) const;
    void setUniformInternal(const GLint location, const glm::ivec3 &data) const;
    void setUniformInternal(const GLint location, const glm::ivec4 &data) const;

    void setUniformInternal(const GLint location, const unsigned int &data) const;
    void setUniformInternal(const GLint location, const glm::uvec2 &data) const;
    void setUniformInternal(const GLint location, const glm::uvec3 &data) const;
    void setUniformInternal(const GLint location, const glm::uvec4 &data) const;

    void setUniformInternal(const GLint location, const glm::mat2 &data) const;
    void setUniformInternal(const GLint location, const glm::mat3 &data) const;
    void setUniformInternal(const GLint location, const glm::mat4 &data) const;

    void setUniformInternal(const GLint location, const glm::mat2x3 &data) const;
    void setUniformInternal(const GLint location, const glm::mat3x2 &data) const;

    void setUniformInternal(const GLint location, const glm::mat2x4 &data) const;
    void setUniformInternal(const GLint location, const glm::mat4x2 &data) const;

    void setUniformInternal(const GLint location, const glm::mat3x4 &data) const;
    void setUniformInternal(const GLint location, const glm::mat4x3 &data) const;

    GLuint m_Program = 0;

    bool m_RequestedReload = false;
    std::function<void(const ResourceWatcherData &)> m_ReloadCallback;

    Info m_Info;

    template <ResourceType T>
    friend class ResourceRegistry;
};

template <>
struct IsResource<ShaderPipeline> : std::true_type {};

} // namespace Physbuzz
