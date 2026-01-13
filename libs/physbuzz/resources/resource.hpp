#pragma once

#include "../debug/macros.hpp"
#include "registry.hpp"

namespace Physbuzz {

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

template <typename T>
struct std::formatter<Physbuzz::Resource<T>> : std::formatter<std::string> {
    auto format(const Physbuzz::Resource<T> &resource, auto &context) const {
        return std::formatter<std::string>::format(resource.getIdentifier(), context);
    }
};

template <typename T>
struct fmt::formatter<Physbuzz::Resource<T>> : fmt::formatter<std::string> {
    auto format(Physbuzz::Resource<T> resource, format_context &ctx) const -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", resource.getIdentifier());
    }
};
