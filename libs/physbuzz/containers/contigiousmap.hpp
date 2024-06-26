#pragma once

#include "../logging.hpp"
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace Physbuzz {

namespace {

template <typename T>
concept Comparable = requires(T a, T b) {
    a <=> b;
};

template <typename F, typename T>
concept Comparator = requires(F f, T &t1, T &t2) {
    { f(t1, t2) } -> std::convertible_to<bool>;
};

} // namespace

template <typename K, typename T>
    requires Comparable<K>
class ContiguousMap {
  public:
    T &operator[](K key) {
        return get(key);
    }

    K insert(T object, K key) {
        if (contains(key)) {
            // overwrite existing mapping if exist
            std::size_t idx = m_IdxMap[key];
            m_Array[idx] = object;
        } else {
            // otherwise, simply append to the end of the array
            m_IdxMap[key] = m_Array.size();
            m_KeyMap[m_Array.size()] = key;
            m_Array.emplace_back(object);
        }

        return key;
    }

    K insert(T object) {
        K key = 0;

        while (contains(key)) {
            key++;
        }

        return insert(object, key);
    }

    bool remove(K key) {
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
        return true;
    }

    template <typename F>
        requires Comparator<F, T>
    void sort(F comparator) {
        if (m_Array.size() <= 1) {
            return;
        }

        // using a custom quicksort to keep the internal maps valid
        quickSort(0, m_Array.size() - 1, comparator);
    }

    bool contains(K key) const {
        return m_IdxMap.contains(key);
    }

    T &get(K key) {
        Logger::ASSERT(contains(key), "Map Does Not Contain Key.");
        return m_Array[m_IdxMap[key]];
    }

    void clear() {
        m_Array.clear();
        m_IdxMap.clear();
        m_KeyMap.clear();
    }

    bool empty() const {
        return m_Array.empty();
    }

    std::size_t size() const {
        return m_Array.size();
    }

    std::vector<T> &getArray() {
        return m_Array;
    }

  protected:
    std::unordered_map<K, std::size_t> m_IdxMap;
    std::unordered_map<std::size_t, K> m_KeyMap;
    std::vector<T> m_Array;

  private:
    template <typename F>
        requires Comparator<F, T>
    void quickSort(int low, int high, F comparator) {
        if (low >= high) {
            return;
        }

        int pivot = partition(low, high, comparator);

        quickSort(low, pivot - 1, comparator);
        quickSort(pivot + 1, high, comparator);
    }

    template <typename F>
        requires Comparator<F, T>
    int partition(int low, int high, F comparator) {
        int pivot = high;
        int i = low - 1;

        for (int j = low; j <= high; ++j) {
            if (comparator(m_Array[j], m_Array[pivot])) {
                ++i;
                swap(i, j);
            }
        }

        swap(++i, high);
        return i;
    }

    void swap(int idx1, int idx2) {
        Logger::ASSERT(idx1 >= 0 && idx1 < m_Array.size() && idx2 >= 0 && idx2 < m_Array.size(), "Invalid swap indices");

        std::iter_swap(m_Array.begin() + idx1, m_Array.begin() + idx2);

        K key1 = m_KeyMap[idx1];
        K key2 = m_KeyMap[idx2];

        m_IdxMap[key1] = idx2;
        m_IdxMap[key2] = idx1;

        m_KeyMap[idx1] = key2;
        m_KeyMap[idx2] = key1;
    }
};

} // namespace Physbuzz
