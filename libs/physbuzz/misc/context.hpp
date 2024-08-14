#pragma once

#include "../debug/logging.hpp"

namespace Physbuzz {

class Context {
  public:
    template <typename T>
    static void set(T *context) {
        storage = context;
    }

    template <typename T>
    static T *get() {
        T *object = static_cast<T *>(storage);
        Logger::ASSERT(object != nullptr, "Context does not exist");
        return object;
    }

  private:
    static void *storage;

    Context() = delete;
    Context(const Context &) = delete;
    Context(const Context &&) = delete;
    ~Context() = delete;
};

} // namespace Physbuzz
