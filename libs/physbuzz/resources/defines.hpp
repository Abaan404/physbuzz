#pragma once

#include <concepts>
#include <efsw/efsw.hpp>
#include <filesystem>
#include <string>

namespace Physbuzz {

using ResourceID = std::string;

enum class WatchAction {
    Add = efsw::Action::Add,
    Delete = efsw::Action::Delete,
    Modified = efsw::Action::Modified,
    Moved = efsw::Action::Moved,
};

struct ResourceWatcherInfo {
    WatchAction action;
    ResourceID identifier;
    std::filesystem::path path;
};

template <typename T>
concept ResourceType = requires(T a) {
    { a.build() } -> std::same_as<bool>;
    { a.destroy() } -> std::same_as<bool>;
};

} // namespace Physbuzz
