#pragma once

#include "../containers/contigiousmap.hpp"
#include <string>

namespace Physbuzz {

template <typename T>
concept ResourceType = requires(T a) {
    { a.build() } -> std::same_as<bool>;
    { a.destroy() } -> std::same_as<bool>;
};

template <ResourceType T>
class ResourceContainer {
  public:
    inline bool build(const std::string &identifier) {
        return base_build(identifier);
    }

    inline bool destroy(const std::string &identifier) {
        return base_destroy(identifier);
    }

    inline bool insert(const std::string &identifier, T &&data) {
        return base_insert(identifier, std::move(data));
    }

    inline bool erase(const std::string &identifier) {
        return base_erase(identifier);
    }

    inline bool contains(const std::string &identifier) {
        return base_contains(identifier);
    }

    inline T *get(const std::string &identifier) {
        return base_get(identifier);
    }

    inline void clear() {
        return base_clear();
    }

  private:
    inline bool base_build(const std::string &identifier) {
        T *resource = get(identifier);
        if (!resource) {
            Logger::ERROR("[ResourceManager] Failed to find resource {}", identifier);
            return false;
        }

        if (!resource->build()) {
            Logger::ERROR("[ResourceManager] Failed to build resource {}", identifier);
            return false;
        }

        return true;
    }

    inline bool base_destroy(const std::string &identifier) {
        T *resource = get(identifier);

        if (!resource) {
            Logger::ERROR("[ResourceManager] Failed to find resource {}", identifier);
            return false;
        }

        if (!resource->destroy()) {
            Logger::ERROR("[ResourceManager] Failed to destroy resource {}", identifier);
            return false;
        }

        return true;
    }

    inline bool base_insert(const std::string &identifier, T &&resource) {
        if (contains(identifier)) {
            erase(identifier);
        }

        m_Map.insert(identifier, resource);
        build(identifier);
        return true;
    }

    inline bool base_erase(const std::string &identifier) {
        destroy(identifier);
        return m_Map.erase(identifier);
    }

    inline bool base_contains(const std::string &identifier) {
        return m_Map.contains(identifier);
    }

    inline T *base_get(const std::string &identifier) {
        if (!contains(identifier)) {
            return nullptr;
        }

        return &m_Map.get(identifier);
    }

    inline void base_clear() {
        for (auto &identifier : m_Map.getKeys()) {
            destroy(identifier);
        }
    }

    ContiguousMap<std::string, T> m_Map;
};

class ResourceRegistry {
  public:
    template <ResourceType T>
    inline static void insert(const std::string &identifier, T &&data) {
        getContainer<T>().insert(identifier, std::move(data));
    }

    template <ResourceType T>
    inline static bool erase(const std::string &identifier) {
        return getContainer<T>().erase(identifier);
    }

    template <ResourceType T>
    inline static T *get(const std::string &identifier) {
        return getContainer<T>().get(identifier);
    }

    template <ResourceType T>
    inline static bool contains(const std::string &identifier) {
        return getContainer<T>().contains(identifier);
    }

    template <ResourceType T>
    inline static void clear() {
        getContainer<T>().clear();
    }

  private:
    template <ResourceType T>
    inline static ResourceContainer<T> &getContainer() {
        static ResourceContainer<T> container = ResourceContainer<T>();
        return container;
    }
};

} // namespace Physbuzz
