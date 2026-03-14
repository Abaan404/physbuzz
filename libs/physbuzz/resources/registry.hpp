#pragma once

#include "../events/handler.hpp"
#include "../events/resources.hpp"
#include "../io/logging.hpp"
#include "defines.hpp"

namespace Physbuzz {

template <ResourceType T>
class ResourceRegistry {
  public:
    template <typename... Args>
        requires(sizeof...(Args) == 0 || ResourceBuildableType<T, Args...>)
    inline static bool insert(const ResourceID &identifier, T &&resource, Args... args) {
        if (contains(identifier)) {
            Logger::ERROR("[ResourceRegistry] resource '{}' was already loaded.", identifier);
            return false;
        }

        if constexpr (ResourceBuildableType<T, Args...>) {
            if (!resource.build(std::forward<Args>(args)...)) {
                Logger::ERROR("[ResourceRegistry] Failed to build resource '{}'.", identifier);
                return false;
            }
        }

        m_Registry.emplace(identifier, std::move(resource));

        Events.notifyCallbacks<OnResourceBuild>({
            .identifier = identifier,
        });

        return true;
    }

    template <typename... Args>
        requires(sizeof...(Args) == 0 || ResourceDestructibleType<T, Args...>)
    inline static bool erase(const ResourceID &identifier, Args... args) {
        if (!contains(identifier)) {
            Logger::ERROR("[ResourceRegistry] resource '{}' was already unloaded or not found.", identifier);
            return false;
        }

        T &resource = m_Registry.at(identifier);

        Events.notifyCallbacks<OnResourceDestroy>({
            .identifier = identifier,
        });

        if constexpr (ResourceDestructibleType<T, Args...>) {
            if (!resource.destroy(std::forward<Args>(args)...)) {
                Logger::ERROR("[ResourceRegistry] Failed to destroy resource '{}'.", identifier);
                return false;
            }
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

    inline static void watch()
        requires IsResourceFilesystem<T>
    {

        if (m_WatchID != -1) {
            return;
        }

        m_WatchID = m_Watcher.addWatch(m_ResourceDirectory, &m_Listener, true);
        m_Watcher.watch();
    }

    inline static bool setResourceDirectory(const std::filesystem::path &directory)
        requires IsResourceFilesystem<T>
    {

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

    inline static const std::filesystem::path &getResourceDirectory()
        requires IsResourceFilesystem<T>
    {
        return m_ResourceDirectory;
    }

    static inline EventSubject Events;

    inline static std::mutex ReloadMutex;

  private:
    inline static std::unordered_map<ResourceID, T> m_Registry;
    inline static std::filesystem::path m_ResourceDirectory = std::filesystem::current_path() / "resources";

    inline static efsw::WatchID m_WatchID = -1;
    inline static efsw::FileWatcher m_Watcher;
    inline static class : public efsw::FileWatchListener {
      public:
        void handleFileAction(efsw::WatchID id, const std::string &directory, const std::string &filename, efsw::Action action, std::string) override {
            if constexpr (!IsResourceFilesystem<T>) {
                return;
            }

            if (id != m_WatchID) {
                return;
            }

            for (auto &[identifier, resource] : m_Registry) {
                if (resource.reload(static_cast<WatchAction>(action), directory + filename)) {
                    Events.notifyCallbacks<OnResourceReload>({
                        .identifier = identifier,
                    });
                }
            }
        }
    } m_Listener;

    template <typename>
    friend class Resource;
};

} // namespace Physbuzz
