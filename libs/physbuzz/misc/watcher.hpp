#pragma once

#include <cstdint>
#include <efsw/efsw.hpp>
#include <filesystem>
#include <functional>

namespace Physbuzz {

using WatchID = std::uint32_t;
using WatchAction = efsw::Action;

struct FileWatcherInfo {
    WatchID id;
    std::filesystem::path path;
    efsw::Action action;
};

class DirectoryListener : public efsw::FileWatchListener {
  public:
    void handleFileAction(efsw::WatchID watchid, const std::string &dir, const std::string &filename, efsw::Action action, std::string oldFilename) override;

  private:
    inline static std::unordered_map<WatchID, std::function<void(const FileWatcherInfo &)>> m_Callbacks;
    friend class FileWatcher;
};

class FileWatcher {
  public:
    static WatchID insert(const std::function<void(const FileWatcherInfo &)> &callback);
    static bool erase(WatchID id);
    static bool contains(WatchID id);

  private:
    inline static WatchID m_IdCounter = 0;
    inline static efsw::FileWatcher m_Watcher;
    inline static DirectoryListener m_Listener;
};

} // namespace Physbuzz
