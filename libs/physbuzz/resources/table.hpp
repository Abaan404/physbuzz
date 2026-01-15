#pragma once

#include "physbuzz/debug/macros.hpp"
#include "resource.hpp"
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
        auto it = m_Resources.erase(resource);

        if (it == m_Resources.end()) {
            return false;
        }

        m_EmptyIndices.push_front(it);
    }

    bool contains(const Resource<T> &resource) const {
        return m_Resources.contains(resource);
    }

    std::uint32_t query(const Resource<T> &resource) const {
        PBZ_ASSERT(m_Resources.contains(resource), std::format("[ResourceTable] Table does not contain {}", resource));
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
    std::unordered_map<Resource<T>, std::uint32_t> m_Resources;
    std::deque<std::uint32_t> m_EmptyIndices;
    std::uint32_t m_ResourceCounter = 0;
};

} // namespace Physbuzz
