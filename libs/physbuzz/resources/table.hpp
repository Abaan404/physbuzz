#pragma once

#include "registry.hpp"
#include <cstdint>
#include <deque>
#include <unordered_map>

namespace Physbuzz {

template <typename T>
class ResourceTable {
  public:
    bool add(const Resource<T> &resource) {
        if (m_Resources.contains(resource)) {
            return false;
        }

        if (!m_EmptyIndices.empty()) {
            m_Resources[resource] = m_EmptyIndices.back();
            m_EmptyIndices.pop_back();
        } else {
            m_Resources[resource] = m_ResourceCounter++;
        }

        return true;
    }

    bool remove(const Resource<T> &resource) {
        auto it = m_Resources.find(resource);
        if (it == m_Resources.end()) {
            return false;
        }

        if (it->second == m_ResourceCounter - 1) {
            m_ResourceCounter--;
        }

        m_EmptyIndices.push_front(it->second);
        m_Resources.erase(it);
        return true;
    }

    const std::unordered_map<ResourceID, std::uint32_t> &getResources() const {
        return m_Resources;
    }

    bool contains(const Resource<T> &resource) const {
        return m_Resources.contains(resource);
    }

    std::uint32_t query(const Resource<T> &resource) const {
        if (!m_Resources.contains(resource)) {
            return static_cast<std::uint32_t>(-1);
        }

        return m_Resources.at(resource);
    }

    std::size_t max_size() {
        return m_ResourceCounter;
    }

    std::size_t size() {
        return m_Resources.size();
    }

    void clear() {
        m_ResourceCounter = 0;
        m_EmptyIndices.clear();
        m_Resources.clear();
    }

  private:
    std::unordered_map<ResourceID, std::uint32_t> m_Resources;
    std::deque<std::uint32_t> m_EmptyIndices;
    std::uint32_t m_ResourceCounter = 0;
};

} // namespace Physbuzz
