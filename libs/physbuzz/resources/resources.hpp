#pragma once

#include "../debug/logging.hpp"
#include "../debug/macros.hpp"
#include "../events/handler.hpp"
#include "../events/resources.hpp"
#include "defines.hpp"
#include <glm/detail/type_quat.hpp>

namespace Physbuzz {

template <ResourceType T>
class ResourceRegistry {
  public:
    template <typename... Args>
        requires ResourceBuildableType<T, Args...>
    inline static bool insert(const ResourceID &identifier, T &&resource, Args... args) {
        if (contains(identifier)) {
            Logger::ERROR("[ResourceRegistry] resource '{}' was already loaded.", identifier);
            return false;
        }

        if (!resource.build(std::forward<Args>(args)...)) {
            Logger::ERROR("[ResourceRegistry] Failed to build resource '{}'.", identifier);
            return false;
        }

        m_Registry.emplace(identifier, std::move(resource));

        Events.notifyCallbacks<OnResourceBuild>({
            .identifier = identifier,
        });

        return true;
    }

    inline static bool erase(const ResourceID &identifier) {
        if (!contains(identifier)) {
            Logger::ERROR("[ResourceRegistry] resource '{}' was already unloaded or not found.", identifier);
            return false;
        }

        T &resource = m_Registry.at(identifier);

        Events.notifyCallbacks<OnResourceDestroy>({
            .identifier = identifier,
        });

        if (!resource.destroy()) {
            Logger::ERROR("[ResourceRegistry] Failed to destroy resource '{}'.", identifier);
            return false;
        }

        m_Registry.erase(identifier);

        return true;
    }

    inline static bool contains(const ResourceID &identifier) {
        return m_Registry.contains(identifier);
    }

    inline static void clear() {
        std::vector<ResourceID> keys;
        keys.reserve(m_Registry.size());

        for (const auto &[id, value] : m_Registry) {
            keys.emplace_back(id);
        }

        for (const auto &id : keys) {
            erase(id);
        }

        m_Registry.clear();
        Events.clearCallbacks();
    }

    inline static void watch() {
        if (m_WatchID != -1) {
            return;
        }

        m_WatchID = m_Watcher.addWatch(m_ResourceDirectory, &m_Listener, true);
        m_Watcher.watch();
    }

    inline static bool setResourceDirectory(const std::filesystem::path &directory) {
        if (!std::filesystem::is_directory(directory)) {
            Logger::ERROR("[ResourceRegistry] path {} is not a directory.", directory.string());
            return false;
        }

        m_ResourceDirectory = directory;

        if (m_WatchID != -1) {
            m_Watcher.removeWatch(m_WatchID);
            m_WatchID = m_Watcher.addWatch(directory, &m_Listener, true);
        }

        return true;
    }

    inline static const std::filesystem::path &getResourceDirectory() {
        return m_ResourceDirectory;
    }

    static inline EventSubject Events;

  private:
    inline static std::unordered_map<ResourceID, T> m_Registry;
    inline static std::filesystem::path m_ResourceDirectory = std::filesystem::current_path() / "resources";

    inline static efsw::WatchID m_WatchID = -1;
    inline static efsw::FileWatcher m_Watcher;
    inline static class : public efsw::FileWatchListener {
      public:
        void handleFileAction(efsw::WatchID, const std::string &directory, const std::string &filename, efsw::Action action, std::string) override {
            for (const auto &[identifier, _] : m_Registry) {
                Events.notifyCallbacks<OnResourceReload>({
                    .identifier = identifier,
                    .filePath = directory + filename,
                    .action = static_cast<WatchAction>(action),
                });
            }
        }
    } m_Listener;

    template <typename>
    friend class Resource;
};

template <typename T>
class Resource {
  public:
    Resource(const ResourceID &identifier)
        : m_Identifier(identifier) {}

    T *operator->() const
        requires ResourceType<T>
    {
        return &get();
    }

    operator const ResourceID &() const {
        return m_Identifier;
    }

    operator T &() {
        return get();
    }

    bool operator==(const Resource<T> &other) const {
        return m_Identifier == other.m_Identifier;
    }

    T &get() const
        requires ResourceType<T>
    {
        PBZ_ASSERT(ResourceRegistry<T>::contains(m_Identifier), std::format("[Resource] Resource '{}' does not exist for type", m_Identifier));
        return ResourceRegistry<T>::m_Registry.at(m_Identifier);
    }

    const ResourceID &getIdentifier() const {
        return m_Identifier;
    }

  private:
    ResourceID m_Identifier;
};

} // namespace Physbuzz

template <typename T>
struct std::hash<Physbuzz::Resource<T>> {
    std::size_t operator()(const Physbuzz::Resource<T> &resource) const noexcept {
        return std::hash<std::string>{}(resource.getIdentifier());
    }
};
