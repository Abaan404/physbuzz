#pragma once

#include "../misc/watcher.hpp"
#include "../resources/file.hpp"
#include "../resources/manager.hpp"
#include "../ecs/scene.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>

namespace Physbuzz {

class ShaderPipelineResource;

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

enum class ShaderType {
    Vertex = GL_VERTEX_SHADER,
    TessControl = GL_TESS_CONTROL_SHADER,
    TessEvaluation = GL_TESS_EVALUATION_SHADER,
    Geometry = GL_GEOMETRY_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    Compute = GL_COMPUTE_SHADER,
    Unknown = GL_INVALID_ENUM,
};

struct ShaderInfo {
    FileInfo file;
};

class Shader {
  public:
    Shader(const ShaderInfo &info, const ShaderType &type);
    ~Shader();

    bool build();
    bool destroy();

    bool compile();

    bool attach(GLuint program) const;
    bool detach(GLuint program) const;

    const GLuint &getShader() const;
    const ShaderType &getType() const;
    const std::set<std::filesystem::path> getIncludedPaths() const;

  private:
    void preprocess(FileResource &file);
    bool preprocessInclude(FileResource &file, std::size_t position);

    std::set<std::filesystem::path> m_IncludedPaths;
    ShaderInfo m_Info;
    GLuint m_Shader = 0;
    ShaderType m_Type = ShaderType::Unknown;
};

struct ShaderPipelineInfo {
    ShaderInfo vertex;
    ShaderInfo tessControl;
    ShaderInfo tessEvaluation;
    ShaderInfo geometry;
    ShaderInfo fragment;
    ShaderInfo compute;

    // (TODO this should be an event for every resource)
    std::function<void(ShaderPipelineResource *)> setup;
    std::function<void(Scene &, ObjectID)> draw;
};

class ShaderPipelineResource {
  public:
    ShaderPipelineResource(const ShaderPipelineInfo &info);
    ~ShaderPipelineResource();

    bool build();
    bool destroy();

    bool reload();
    void requestReload();

    bool bind() const;
    bool unbind() const;

    template <UniformType T>
    inline void setUniform(const std::string &name, const T &data) const {
        setUniformInternal(glGetUniformLocation(m_Program, name.c_str()), data);
    }

    const GLuint &getProgram() const;

    std::function<void(Scene &, ObjectID)> draw;

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

    ShaderPipelineInfo m_Info;
    GLuint m_Program = 0;
    WatchID m_WatchId = 0;

    bool m_RequestedReload = false;
    std::set<std::filesystem::path> m_Paths;

    template <ResourceType T>
    friend class ResourceContainer;
};

template <>
bool ResourceContainer<ShaderPipelineResource>::insert(const std::string &identifier, ShaderPipelineResource &&resource);

template <>
bool ResourceContainer<ShaderPipelineResource>::erase(const std::string &identifier);

} // namespace Physbuzz
