#pragma once

namespace Physbuzz {

class Context {
  public:
    template <typename T>
    static void set(T *context) {
        storage = context;
    }

    template <typename T>
    static T *get() {
        return static_cast<T *>(storage);
    }

  private:
    static void *storage;

    Context() = delete;
    Context(const Context &) = delete;
    Context(const Context &&) = delete;
    ~Context() = delete;
};

} // namespace Physbuzz
