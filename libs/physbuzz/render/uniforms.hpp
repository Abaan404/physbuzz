#pragma once

#include "../debug/logging.hpp"
#include "../resources/defines.hpp"
#include <glad/gl.h>
#include <type_traits>

namespace Physbuzz {

template <typename T>
concept UniformBufferType =
    std::is_class_v<T> &&
    std::is_trivial_v<T> &&
    std::is_standard_layout_v<T>;

template <UniformBufferType T>
class UniformBuffer {
  public:
    bool build() {
        if (UBO != 0) {
            Logger::WARNING("[UniformBuffer] Trying to build a constructed uniform.");
            return true;
        }

        glCreateBuffers(1, &UBO);
        glNamedBufferData(UBO, sizeof(T), nullptr, GL_STREAM_DRAW);

        return true;
    }

    bool destroy() {
        if (UBO == 0) {
            Logger::WARNING("[UniformBuffer] Trying to destroy a destructed uniform.");
            return true;
        }

        glDeleteBuffers(1, &UBO);
        return true;
    }

    void update(const T &data) const {
        glNamedBufferSubData(UBO, 0, sizeof(T), &data);
    }

    void bindPipeline(GLuint binding) const {
        PBZ_ASSERT(UBO != 0, "[UniformBuffer] trying to bind an incomplete uniform buffer to a pipeline.");
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, UBO);
    }

    void unbindPipeline(GLuint binding) const {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0);
    }

  private:
    GLuint UBO = 0;
};

template <UniformBufferType T>
struct IsResource<UniformBuffer<T>> : std::true_type {};

} // namespace Physbuzz
