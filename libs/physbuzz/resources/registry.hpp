#pragma once

#include "../events/handler.hpp"
#include "../events/resources.hpp"
#include "../io/logging.hpp"
#include "defines.hpp"
#include <optional>
#include <unordered_set>
#include <utility>

namespace Physbuzz {

template <typename T>
class Resource {
  public:
    Resource();
    Resource(const ResourceID &id);
    ~Resource();

    Resource(const Resource &);
    Resource &operator=(const Resource &);

    Resource(Resource &&);
    Resource &operator=(Resource &&);

    T *operator->() const
        requires ResourceType<T>
    {
        return &get();
    }

    operator T &() {
        return get();
    }

    operator const ResourceID &() const {
        return getId();
    }

    operator const std::string_view() const {
        return getName();
    }

    T &get() const;

    const ResourceID &getId() const {
        return m_Id;
    }

    const std::string_view getName() const;

  private:
    ResourceID m_Id = ResourceEmptyID;
};

template <ResourceType T>
class ResourceRegistry {
  public:
    template <typename... Args>
        requires(sizeof...(Args) == 0 || ResourceBuildableType<T, Args...>)
    static std::optional<Resource<T>> insert(const std::string &name, T &&resource, Args... args) {
        if (contains(name)) {
            Logger::ERROR("[ResourceRegistry] Resource '{}' already exists.", name);
            return std::nullopt;
        }

        if constexpr (ResourceBuildableType<T, Args...>) {
            if (!resource.build(std::forward<Args>(args)...)) {
                Logger::ERROR("[ResourceRegistry] Failed to build resource '{}'.", name);
                return std::nullopt;
            }
        }

        ResourceID id = ResourceEmptyID;

        if (!m_FreedResources.empty()) {
            id = m_FreedResources.extract(m_FreedResources.begin()).value();

            m_Resources[id] = std::move(resource);
            m_Names[id] = name;
            m_RefCounts[id] = 0;
        } else {
            id = m_Resources.size();

            m_Resources.emplace_back(std::move(resource));
            m_Names.emplace_back(name);
            m_RefCounts.emplace_back(0);
        }

        m_NameLookup[name] = id;

        m_PendingCallbacks.insert(id);

        Events.notifyCallbacks<OnResourceBuild>({
            .id = id,
        });

        m_PendingCallbacks.erase(id);

        return std::make_optional<Resource<T>>(id);
    }

    static std::optional<Resource<T>> get(const std::string &name) {
        if (!contains(name)) {
            return std::nullopt;
        }

        return std::make_optional<Resource<T>>(m_NameLookup.at(name));
    }

    static std::uint32_t getRefCount(const std::string &name) {
        if (!contains(name)) {
            return 0;
        }

        return getRefCount(m_NameLookup[name]);
    }

    static std::uint32_t getRefCount(const ResourceID &id) {
        if (!contains(id)) {
            return 0;
        }

        return m_RefCounts.at(id);
    }

    inline static bool contains(const std::string &name) {
        return m_NameLookup.contains(name);
    }

    inline static bool contains(const ResourceID id) {
        return id < m_Resources.size() && !m_FreedResources.contains(id);
    }

    inline static void clear() {
        if constexpr (ResourceDestructibleType<T>) {
            for (ResourceID id = 0; id < m_Resources.size(); id++) {
                if (m_FreedResources.contains(id)) {
                    continue;
                }

                if (!m_Resources[id].destroy()) {
                    Logger::ERROR("[ResourceRegistry] Failed to destroy resource '{}'.", m_Names[id]);
                }
            }
        }

        for (std::size_t id = 0; id < m_Resources.size(); id++) {
            if (m_RefCounts[id] > 0) {
                Logger::WARNING("[ResourceRegistry] resource '{}' still has {} live references.", m_Names[id], m_RefCounts[id]);
            }
        }

        m_Resources.clear();
        m_Names.clear();
        m_RefCounts.clear();
        m_PendingCallbacks.clear(); // technically not needed but just in case

        m_NameLookup.clear();
        m_FreedResources.clear();

        Events.clearCallbacks();
    }

    inline static void watch()
        requires IsResourceFilesystem<T>
    {

        if (m_WatchID != -1) {
            return;
        }

        m_WatchID = m_Watcher.addWatch(m_Directory, &m_Listener, true);
        m_Watcher.watch();
    }

    inline static bool setResourceDirectory(const std::filesystem::path &directory)
        requires IsResourceFilesystem<T>
    {

        if (!std::filesystem::is_directory(directory)) {
            Logger::ERROR("[ResourceRegistry] path {} is not a directory.", directory.string());
            return false;
        }

        m_Directory = directory;

        if (m_WatchID != -1) {
            m_Watcher.removeWatch(m_WatchID);
            m_WatchID = m_Watcher.addWatch(directory, &m_Listener, true);
        }

        return true;
    }

    inline static const std::filesystem::path &getResourceDirectory()
        requires IsResourceFilesystem<T>
    {
        return m_Directory;
    }

    static inline EventSubject Events;

    inline static std::mutex ReloadMutex;

  private:
    static void erase(const ResourceID id) {
        if (!contains(id)) {
            Logger::ERROR("[ResourceRegistry] resource id '{}' was already unloaded or not found.", id);
            return;
        }

        if (m_RefCounts[id] > 0) {
            Logger::ERROR("[ResourceRegistry] resource '{}' still has {} live references.", m_Names[id], m_RefCounts[id] - 1);
            return;
        }

        // callbacks can create new references, make sure erase doesnt happen twice
        if (m_PendingCallbacks.contains(id)) {
            return;
        }

        m_PendingCallbacks.insert(id);

        Events.notifyCallbacks<OnResourceDestroy>({
            .id = id,
        });

        m_PendingCallbacks.erase(id);

        // callbacks can still leave dangling refs
        if (m_RefCounts[id] > 0) {
            Logger::ERROR("[ResourceRegistry] resource '{}' still has {} live references.", m_Names[id], m_RefCounts[id] - 1);
            return;
        }

        if constexpr (ResourceDestructibleType<T>) {
            T &resource = m_Resources[id];
            if (!resource.destroy()) {
                Logger::ERROR("[ResourceRegistry] Failed to destroy resource '{}'.", m_Names[id]);
                return;
            }
        }

        m_NameLookup.erase(m_Names[id]);

        if (id != m_Resources.size() - 1) {
            // m_Resources[id] remains untouched but marked deleted
            m_FreedResources.insert(id);

            m_Names[id] = "";
            m_RefCounts[id] = 0;
        } else {
            m_Resources.pop_back();
            m_Names.pop_back();
            m_RefCounts.pop_back();
        }
    }

    static void decrementRef(const ResourceID id) {
        if (!contains(id)) {
            return;
        }

        m_RefCounts[id]--;

        if (m_RefCounts[id] == 0) {
            erase(id);
        }
    }

    static void incrementRef(const ResourceID id) {
        if (!contains(id)) {
            return;
        }

        m_RefCounts[id]++;
    }

    inline static std::vector<T> m_Resources;
    inline static std::vector<std::string> m_Names;
    inline static std::vector<std::uint32_t> m_RefCounts;
    inline static std::unordered_set<ResourceID> m_PendingCallbacks;

    inline static std::unordered_map<std::string, ResourceID> m_NameLookup;
    inline static std::unordered_set<ResourceID> m_FreedResources; // Note: maybe require resources to signal they are ready, making this set not necessary

    inline static std::filesystem::path m_Directory = std::filesystem::current_path() / "resources";

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

            for (ResourceID id = 0; id < m_Resources.size(); id++) {
                if (m_FreedResources.contains(id)) {
                    continue;
                }

                if (m_Resources[id].reload(static_cast<WatchAction>(action), directory + filename)) {
                    Events.notifyCallbacks<OnResourceReload>({
                        .id = id,
                    });
                }
            }
        }
    } m_Listener;

    template <typename>
    friend class Resource;

    template <typename>
    friend class ResourceRef;
};

template <typename T>
inline Resource<T>::Resource() {}

template <typename T>
inline Resource<T>::Resource(const ResourceID &id)
    : m_Id(id) {
    ResourceRegistry<T>::incrementRef(m_Id);
}

template <typename T>
inline Resource<T>::~Resource() {
    ResourceRegistry<T>::decrementRef(m_Id);
}

template <typename T>
inline Resource<T>::Resource(const Resource &other)
    : m_Id(other.m_Id) {
    ResourceRegistry<T>::incrementRef(m_Id);
}

template <typename T>
inline Resource<T> &Resource<T>::operator=(const Resource &other) {
    if (this == &other) {
        return *this;
    }

    ResourceRegistry<T>::decrementRef(m_Id);
    ResourceRegistry<T>::incrementRef(other.m_Id);

    m_Id = other.m_Id;
    return *this;
}

template <typename T>
inline Resource<T>::Resource(Resource &&other)
    : m_Id(std::exchange(other.m_Id, ResourceEmptyID)) {}

template <typename T>
inline Resource<T> &Resource<T>::operator=(Resource &&other) {
    if (this == &other) {
        return *this;
    }

    ResourceRegistry<T>::decrementRef(m_Id);
    m_Id = std::exchange(other.m_Id, ResourceEmptyID);

    return *this;
}

template <typename T>
inline T &Resource<T>::get() const {
    return ResourceRegistry<T>::m_Resources[m_Id];
}

template <typename T>
inline const std::string_view Resource<T>::getName() const {
    return ResourceRegistry<T>::m_Names.at(m_Id);
}

} // namespace Physbuzz

template <typename T>
struct std::hash<Physbuzz::Resource<T>> {
    std::size_t operator()(const Physbuzz::Resource<T> &resource) const noexcept {
        return std::hash<Physbuzz::ResourceID>{}(resource.getId());
    }
};

template <typename T>
struct std::formatter<Physbuzz::Resource<T>> : std::formatter<std::string_view> {
    auto format(const Physbuzz::Resource<T> &resource, auto &context) const {
        return std::formatter<std::string_view>::format(resource.getName(), context);
    }
};

template <typename T>
struct fmt::formatter<Physbuzz::Resource<T>> : fmt::formatter<std::string_view> {
    auto format(Physbuzz::Resource<T> resource, format_context &ctx) const -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", resource.getName());
    }
};
