#include "watcher.hpp"

namespace Physbuzz {

void DirectoryListener::handleFileAction(efsw::WatchID watchid, const std::string &dir, const std::string &filename, efsw::Action action, std::string oldFilename) {
    for (const auto &[id, callback] : m_Callbacks) {
        callback({
            .id = id,
            .path = dir + filename,
            .action = action,
        });
    }
}

WatchID FileWatcher::insert(const std::function<void(const FileWatcherInfo &)> &callback) {
    static auto runOnce = []() {
        m_Watcher.allowOutOfScopeLinks(true);
        m_Watcher.followSymlinks(true);
        m_Watcher.addWatch(std::filesystem::current_path(), &m_Listener, true);
        m_Watcher.watch();
        return 0;
    }();

    DirectoryListener::m_Callbacks[m_IdCounter++] = callback;
    return m_IdCounter;
}

bool FileWatcher::erase(WatchID id) {
    return DirectoryListener::m_Callbacks.erase(id);
}

bool FileWatcher::contains(WatchID id) {
    return DirectoryListener::m_Callbacks.contains(id);
}

} // namespace Physbuzz
