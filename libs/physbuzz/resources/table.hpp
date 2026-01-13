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
    bool add(const Resource<T> &id) {
        if (m_Resources.contains(id)) {
            return false;
        }

        if (!m_EmptyIndices.empty()) {
            m_Resources[id] = m_EmptyIndices.back();
            m_EmptyIndices.pop_back();
        } else {
            m_Resources[id] = m_ResourceCounter++;
        }

        return true;
    }

    bool remove(const Resource<T> &id) {
        auto it = m_Resources.erase(id);

        if (it == m_Resources.end()) {
            return false;
        }

        m_EmptyIndices.push_front(it);
    }

    std::uint32_t query(const Resource<T> id) const {
        PBZ_ASSERT(m_Resources.contains(id), std::format("[ResourceTable] Table does not contain {}", id));
        return m_Resources.at(id);
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
