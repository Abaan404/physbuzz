#pragma once

#include "../containers/contigiousmap.hpp"
#include <string>

namespace Physbuzz {

template <typename T>
concept ResourceType = requires(T a) {
    { a.build() } -> std::same_as<void>;
    { a.destroy() } -> std::same_as<void>;
};

template <ResourceType T>
class ResourceContainer {
  public:
    void build() {
        base_build();
    }

    void destroy() {
        base_destroy();
    }

    void insert(const std::string &identifer, const T &data) {
        base_insert(identifer, data);
    }

    bool remove(const std::string &identifer) {
        return base_remove(identifer);
    }

    bool contains(const std::string &identifier) {
        return base_contains(identifier);
    }

    T *get(const std::string &identifer) {
        return base_get(identifer);
    }

  private:
    void base_build() {
        for (const auto &identifier : m_Map.getKeys()) {
            m_Map.get(identifier).build();
        }
    }

    void base_destroy() {
        for (const auto &identifier : m_Map.getKeys()) {
            m_Map.get(identifier).destroy();
        }
    }

    void base_insert(const std::string &identifer, const T &resource) {
        m_Map.insert(identifer, resource);
    }

    bool base_remove(const std::string &identifer) {
        return m_Map.erase(identifer);
    }

    bool base_contains(const std::string &identifier) {
        return m_Map.contains(identifier);
    }

    T *base_get(const std::string &identifer) {
        if (m_Map.contains(identifer)) {
            return &m_Map.get(identifer);
        } else {
            return nullptr;
        }
    }

    ContiguousMap<std::string, T> m_Map;
};

class ResourceManager {
  public:
    template <ResourceType T>
    void build() {
        getContainer<T>().build();
    }

    template <ResourceType T>
    void destroy() {
        getContainer<T>().destroy();
    }

    template <ResourceType T>
    T *get(const std::string &identifier) {
        return getContainer<T>().get(identifier);
    }

    template <ResourceType T>
    void insert(const std::string &identifier, const T &data) {
        getContainer<T>().insert(identifier, data);
    }

    template <ResourceType T>
    bool remove(const std::string &identifier) {
        return getContainer<T>().remove(identifier);
    }

    template <ResourceType T>
    bool contains(const std::string &identifier) {
        return getContainer<T>().contains(identifier);
    }

  private:
    template <ResourceType T>
    ResourceContainer<T> &getContainer() {
        static ResourceContainer<T> container = ResourceContainer<T>();
        return container;
    }
};

} // namespace Physbuzz
