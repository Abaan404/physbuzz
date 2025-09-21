#pragma once

#include "../containers/contigiousmap.hpp"
#include "../events/handler.hpp"
#include "../events/resources.hpp"
#include "defines.hpp"
#include <glm/detail/type_quat.hpp>

namespace Physbuzz {

namespace detail {

template <typename T>
concept ResourceWatched = requires(T a) {
    requires std::same_as<decltype(a.m_ReloadCallback), std::function<void(const ResourceWatcherData &)>>;
} && ResourceType<T>;

template <ResourceType T>
class ResourceFileWatcher : public efsw::FileWatchListener {
  public:
    std::unordered_map<ResourceID, std::function<void(const ResourceWatcherData &)>> callbacks;

    void handleFileAction(efsw::WatchID, const std::string &directory, const std::string &filename, efsw::Action action, std::string) override {
        for (const auto &[id, callback] : callbacks) {
            callback({
                .action = static_cast<WatchAction>(action),
                .identifier = id,
                .path = directory + filename,
            });
        }
    }
};

} // namespace detail

template <ResourceType T>
class ResourceRegistry {
  public:
    template <typename... Args>
        requires ResourceBuildableType<T, Args...>
    inline static bool insert(const ResourceID &identifier, T &&resource, Args... args) {
        if (contains(identifier)) {
            Logger::ERROR("[ResourceRegistry] resource \"{}\" was already loaded.", identifier);
            return false;
        }

        if (!resource.build(std::forward<Args>(args)...)) {
            Logger::ERROR("[ResourceRegistry] Failed to build resource \"{}\".", identifier);
            return false;
        }

        m_Container.insert(identifier, std::move(resource));

        if constexpr (detail::ResourceWatched<T>) {
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

    inline static void clear() {
        std::set<ResourceID> keys = m_Container.getKeys();
        for (const auto &id : keys) {
            erase(id);
        }
        m_Container.clear();
        Events.clearCallbacks();
    }

    inline static void watch() {
        m_Watcher.allowOutOfScopeLinks(true);
        m_Watcher.followSymlinks(true);

        if (m_ResourceDirectory.empty()) {
            setResourceDirectory(std::filesystem::current_path()); // use cwd by default
        }

        m_Watcher.watch();
    }

    inline static void setResourceDirectory(const std::filesystem::path &directory) {
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

    inline static detail::ResourceFileWatcher<T> m_Listener;
    inline static efsw::FileWatcher m_Watcher;
    inline static std::filesystem::path m_ResourceDirectory;

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

    bool operator==(const Resource<T> &other) const {
        return m_Identifier == other.m_Identifier;
    }

    T &get() const
        requires ResourceType<T>
    {
        PBZ_ASSERT(ResourceRegistry<T>::contains(m_Identifier), std::format("[Resource] Resource \"{}\" does not exist for type", m_Identifier));
        return ResourceRegistry<T>::m_Container.get(m_Identifier);
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
