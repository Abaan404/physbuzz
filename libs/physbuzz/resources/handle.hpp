#pragma once

#include "../debug/logging.hpp"
#include "defines.hpp"
#include "manager.hpp"
#include <format>

namespace Physbuzz {

template <ResourceType T>
class ResourceHandle {
  public:
    ResourceHandle(const std::string &identifier)
        : m_Identifier(identifier) {}

    T *operator->() const {
        return get();
    }

    T *get() const {
        PBZ_ASSERT(ResourceRegistry<T>::contains(m_Identifier), std::format("[ResourceHandle] Resource \"{}\" does not exist for type", m_Identifier));
        return &ResourceRegistry<T>::m_Container.get(m_Identifier);
    }

    const ResourceID &getIdentifier() const {
        return m_Identifier;
    }

  private:
    ResourceID m_Identifier;
};

} // namespace Physbuzz
