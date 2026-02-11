#pragma once

#include "physbuzz/debug/macros.hpp"
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Physbuzz {

template <typename K>
class DirectedGraph {
  public:
    K insertNode(const K &id) {
        m_Adjacency.try_emplace(id);
        return id;
    }

    std::pair<K, K> insertEdge(const K &from, const K &to) {
        insertNode(from);
        insertNode(to);
        m_Adjacency[from].insert(to);

        return {from, to};
    }

    bool eraseNode(const K &id) {
        if (!m_Adjacency.contains(id)) {
            return false;
        }

        m_Adjacency.erase(id);
        return true;
    }

    bool eraseEdge(const K &from, const K &to) {
        if (!m_Adjacency.contains(from)) {
            return false;
        }

        if (!m_Adjacency.contains(to)) {
            return false;
        }

        m_Adjacency[from].erase(to);
        return true;
    }

    bool containsNode(const K &id) const {
        return m_Adjacency.contains(id);
    }

    void merge(const DirectedGraph<K> &other) {
        for (auto &[node, neighbors] : m_Adjacency) {
            for (const auto &[otherNode, otherNeighbor] : other.m_Adjacency) {
                if (node == otherNode) {
                    neighbors.insert(otherNeighbor.begin(), otherNeighbor.end());
                }
            }
        }
    }

    bool empty() const {
        return m_Adjacency.empty();
    }

    bool size() const {
        return m_Adjacency.size();
    }

    void cull(const K &preserve) {
        // TODO
    }

    void clear() {
        m_Adjacency.clear();
    }

    std::vector<K> sort() const {
        std::unordered_map<K, std::size_t> indegrees;

        for (const auto &[node, neighbors] : m_Adjacency) {
            indegrees.try_emplace(node, 0);
        }

        for (const auto &[node, neighbors] : m_Adjacency) {
            for (const auto &neighbor : neighbors) {
                indegrees.at(neighbor)++;
            }
        }

        std::deque<K> queue;
        for (const auto &[node, indegree] : indegrees) {
            if (indegree == 0) {
                queue.emplace_back(node);
            }
        }

        std::vector<K> result;
        result.reserve(m_Adjacency.size());

        while (!queue.empty()) {
            K current = queue.front();
            queue.pop_front();
            result.emplace_back(current);

            for (const auto &neighbor : m_Adjacency.at(current)) {
                if (--indegrees.at(neighbor) == 0) {
                    queue.emplace_back(neighbor);
                }
            }
        }

        PBZ_ASSERT(result.size() == m_Adjacency.size(), "[DirectedGraph] Cycle detected while sorting topologically");
        return result;
    }

  private:
    std::unordered_map<K, std::unordered_set<K>> m_Adjacency;
};

} // namespace Physbuzz
