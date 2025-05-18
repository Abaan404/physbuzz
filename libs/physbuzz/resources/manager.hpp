#pragma once

#include "../containers/contigiousmap.hpp"
#include "../events/handler.hpp"
#include "../events/resources.hpp"

namespace Physbuzz {

namespace {

template <typename T>
concept ResourceCustomWatched = requires(T a) {
    requires std::same_as<decltype(a.m_ReloadCallback), std::function<void(const ResourceWatcherInfo &)>>;
} && ResourceType<T>;

template <ResourceType T>
class ResourceFileWatcher : public efsw::FileWatchListener {
  public:
    std::unordered_map<ResourceID, std::function<void(const ResourceWatcherInfo &)>> callbacks;

    void handleFileAction(efsw::WatchID _1, const std::string &directory, const std::string &filename, efsw::Action action, std::string _2) override {
        for (const auto &[id, callback] : callbacks) {
            callback({
                .action = static_cast<WatchAction>(action),
                .identifier = id,
                .path = directory + filename,
            });
        }
    }
};

} // namespace

template <ResourceType T>
class ResourceRegistry {
  public:
    inline static bool insert(const ResourceID &identifier, T &&resource) {
        if (contains(identifier)) {
            Logger::ERROR("[ResourceRegistry] resource \"{}\" was already loaded.", identifier);
            return false;
        }

        if (!resource.build()) {
            Logger::ERROR("[ResourceRegistry] Failed to build resource \"{}\".", identifier);
            return false;
        }

        m_Container.insert(identifier, std::move(resource));

        if constexpr (ResourceCustomWatched<T>) {
            m_Listener.callbacks[identifier] = m_Container.get(identifier).m_ReloadCallback;
        }

        Events.notifyCallbacks<OnResourceBuild>({
            .identifier = identifier,
        });

        return true;
    }

    inline static bool erase(const ResourceID &identifier) {
        if (!contains(identifier)) {
            Logger::ERROR("[ResourceRegistry] resource \"{}\" was already unloaded or not found.", identifier);
            return false;
        }

        T &resource = m_Container.get(identifier);

        Events.notifyCallbacks<OnResourceDestroy>({
            .identifier = identifier,
        });

        if (!resource.destroy()) {
            Logger::ERROR("[ResourceRegistry] Failed to destroy resource \"{}\".", identifier);
            return false;
        }

        m_Container.erase(identifier);
        m_Listener.callbacks.erase(identifier);

        return true;
    }

    inline static bool contains(const ResourceID &identifier) {
        return m_Container.contains(identifier);
    }

    static void watch() {
        m_Watcher.allowOutOfScopeLinks(true);
        m_Watcher.followSymlinks(true);

        if (m_ResourceDirectory.empty()) {
            setResourceDirectory(std::filesystem::current_path()); // use cwd by default
        }

        m_Watcher.watch();
    }

    static void setResourceDirectory(const std::filesystem::path &directory) {
        if (!std::filesystem::is_directory(directory)) {
            return;
        }

        m_ResourceDirectory = directory;

        static efsw::WatchID resourcePathWatchId = -1;
        m_Watcher.removeWatch(resourcePathWatchId);
        m_Watcher.addWatch(directory, &m_Listener, true);
    }

    // crappy workaround to allow this static class to generate events through a proxy
    static inline EventSubject Events;

  private:
    inline static ContiguousMap<ResourceID, T> m_Container;

    inline static ResourceFileWatcher<T> m_Listener;
    inline static efsw::FileWatcher m_Watcher;
    inline static std::filesystem::path m_ResourceDirectory;

    template <ResourceType>
    friend class ResourceHandle;
};

} // namespace Physbuzz
