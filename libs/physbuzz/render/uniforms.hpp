#pragma once

#include "../debug/logging.hpp"
#include "shaders.hpp"
#include <glad/gl.h>
#include <type_traits>

namespace Physbuzz {

template <typename T>
concept UniformBufferType =
    std::is_class_v<T> &&
    std::is_trivial_v<T> &&
    std::is_standard_layout_v<T>;

template <UniformBufferType T>
class UniformBufferResource {
  public:
    UniformBufferResource() {}
    ~UniformBufferResource() {}

    bool build() {
        if (UBO != 0) {
            Logger::WARNING("[UniformBufferResource] Trying to build a constructed uniform.");
        }

        glGenBuffers(1, &UBO);

        bind();
        glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_STREAM_DRAW);
        unbind();

        return true;
    }

    bool destroy() {
        if (UBO == 0) {
            Logger::WARNING("[UniformBufferResource] Trying to destroy a destructed uniform.");
        }

        glDeleteBuffers(1, &UBO);
        return true;
    }

    void update(const T &data) {
        bind();
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &data);
        unbind();
    }

    void bind() {
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    }

    void unbind() {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void bindPipeline(const ShaderPipelineResource *pipeline, GLuint binding) {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, UBO);
    }

    void unbindPipeline(const ShaderPipelineResource *pipeline, GLuint binding) {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0); // 0 means unbinding
    }

  private:
    GLuint UBO = 0;
};

} // namespace Physbuzz
