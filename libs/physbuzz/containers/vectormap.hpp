#pragma once

#include "physbuzz/debug.hpp"
#include <unordered_map>
#include <vector>

namespace Physbuzz {

template <typename K, typename T>
class VectorMap {
  public:
    VectorMap() {}

    VectorMap(const VectorMap &other) {
        copy(other);
    }

    VectorMap operator=(const VectorMap &other) {
        return copy(other);
    }

    ~VectorMap() {}

    T &operator[](K key) {
        return get(key);
    }

    K insert(T object, K key) {
        if (contains(key)) {
            // otherwise, overwrite existing mapping
            std::size_t idx = m_IdxMap[key];
            m_Array[idx] = object;
        } else {
            // if not exists, simply append to the end of the array
            m_IdxMap[key] = m_Array.size();
            m_Array.emplace_back(object);
        }

        // to be used for constant lookup during removal
        m_PrevKey = key;
        return key;
    }

    K insert(T object) {
        K key = 0;

        while (contains(key))
            key++;

        return insert(object, key);
    }

    bool remove(K key) {
        // get the component idx from the object
        if (!contains(key)) {
            return false;
        }

        // swap elements and update map (if the idx isnt the end)
        std::size_t idx = m_IdxMap[key];
        if (idx != m_Array.size() - 1) {
            std::iter_swap(m_Array.begin() + idx, m_Array.end() - 1);
            m_IdxMap[m_PrevKey] = idx; // due to stack implementation
            m_PrevKey = key;
        }

        // pop component
        m_IdxMap.erase(key);
        m_Array.pop_back();
        return true;
    }

    bool contains(K key) {
        return m_IdxMap.contains(key);
    }

    T &get(K key) {
        ASSERT(contains(key), "Map Does Not Contain Key.")
        return m_Array[m_IdxMap[key]];
    }

    void clear() {
        m_Array.clear();
        m_IdxMap.clear();
    }

    bool empty() {
        return m_Array.empty();
    }

    std::size_t size() {
        return m_Array.size();
    }

    std::vector<T> &getArray() {
        return m_Array;
    }

  protected:
    K m_PrevKey{0};

    std::unordered_map<K, std::size_t> m_IdxMap;
    std::vector<T> m_Array;

    VectorMap copy(const VectorMap &other) {
        if (this != &other) {
            m_Array = other.m_Array;
            m_IdxMap = other.m_IdxMap;
        }

        return *this;
    }
};

} // namespace Physbuzz
