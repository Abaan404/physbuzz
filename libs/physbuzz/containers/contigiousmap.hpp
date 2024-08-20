#pragma once

#include "physbuzz/debug/logging.hpp"
#include <set>
#include <unordered_map>
#include <vector>

namespace Physbuzz {

namespace {

template <typename T>
concept Comparable = requires(T a, T b) {
    a <=> b;
};

} // namespace

template <Comparable K, typename T>
class ContiguousMap {
  public:
    K insert(const K &key, const T &object) {
        if (contains(key)) {
            // overwrite existing mapping if exist
            std::size_t idx = m_IdxMap[key];
            m_Array[idx] = object;
        } else {
            // otherwise, simply append to the end of the array
            m_IdxMap[key] = m_Array.size();
            m_KeyMap[m_Array.size()] = key;
            m_Array.emplace_back(object);

            // cache the key
            m_Keys.insert(key);
        }

        return key;
    }

    bool erase(const K &key) {
        // get the component idx from the object
        if (!contains(key)) {
            return false;
        }

        // swap elements and update map (if the idx isnt the end)
        std::size_t idx = m_IdxMap[key];
        std::size_t idxEnd = m_Array.size() - 1;
        if (idx != idxEnd) {
            std::iter_swap(m_Array.begin() + idx, m_Array.begin() + idxEnd);

            K keySwapped = m_KeyMap[idxEnd];
            m_IdxMap[keySwapped] = idx;
            m_KeyMap[idx] = keySwapped;
        }

        // pop component
        m_KeyMap.erase(idxEnd);
        m_IdxMap.erase(key);
        m_Array.pop_back();

        // remove the cached key
        m_Keys.erase(key);
        return true;
    }

    bool contains(const K &key) const {
        return m_IdxMap.contains(key);
    }

    T &get(const K &key) {
        PBZ_ASSERT(contains(key), "Map Does Not Contain Key.");
        return m_Array[m_IdxMap[key]];
    }

    void clear() {
        m_Array.clear();
        m_Keys.clear();
        m_IdxMap.clear();
        m_KeyMap.clear();
    }

    bool empty() const {
        return m_Array.empty();
    }

    std::size_t size() const {
        return m_Array.size();
    }

    const std::vector<T> &getArray() {
        return m_Array;
    }

    const std::set<K> &getKeys() {
        return m_Keys;
    }

  protected:
    std::unordered_map<K, std::size_t> m_IdxMap;
    std::unordered_map<std::size_t, K> m_KeyMap;
    std::vector<T> m_Array;
    std::set<K> m_Keys;
};

} // namespace Physbuzz
