#pragma once

#include <concepts>
#include <efsw/efsw.hpp>
#include <filesystem>
#include <string>
#include <type_traits>

namespace Physbuzz {

template <typename T>
struct IsResource : std::false_type {};

using ResourceID = std::string;

enum class WatchAction {
    Add = efsw::Action::Add,
    Delete = efsw::Action::Delete,
    Modified = efsw::Action::Modified,
    Moved = efsw::Action::Moved,
};

template <typename T>
concept ResourceType =
    IsResource<T>::value;

template <typename T, typename... Args>
concept ResourceBuildableType =
    ResourceType<T> &&
    requires(T a, Args... args) {
        { a.build(args...) } -> std::same_as<bool>;
    };

template <typename T, typename... Args>
concept ResourceDestructibleType =
    ResourceType<T> &&
    requires(T a, Args... args) {
        { a.destroy(args...) } -> std::same_as<bool>;
    };

template <typename T>
concept IsResourceFilesystem =
    ResourceType<T> &&
    requires(T a, std::filesystem::path path, WatchAction action) {
        { a.reload(action, path) } -> std::same_as<bool>;
    };

} // namespace Physbuzz
