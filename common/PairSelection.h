#pragma once
#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <queue>
#include <vector>

template <class Candidate>
std::vector<std::size_t> select_unique_pairs(const std::vector<Candidate>& candidates,
                                                   std::size_t lightCount)
{
    std::vector<std::vector<std::size_t>> adjacent(lightCount);
    for (std::size_t k = 0; k < candidates.size(); ++k) {
        adjacent[candidates[k].first].push_back(k);
        adjacent[candidates[k].second].push_back(k);
    }
    std::vector<bool> visited(lightCount, false);
    std::vector<std::size_t> selected;
    for (std::size_t start = 0; start < lightCount; ++start) {
        if (visited[start] || adjacent[start].empty()) continue;
        std::queue<std::size_t> pending;
        std::vector<std::size_t> vertices;
        pending.push(start); visited[start] = true;
        while (!pending.empty()) {
            auto v = pending.front(); pending.pop(); vertices.push_back(v);
            for (auto k : adjacent[v]) {
                auto w = candidates[k].first == v ? candidates[k].second : candidates[k].first;
                if (!visited[w]) { visited[w] = true; pending.push(w); }
            }
        }
        std::vector<std::size_t> componentEdges;
        for (std::size_t k = 0; k < candidates.size(); ++k)
            if (std::find(vertices.begin(), vertices.end(), candidates[k].first) != vertices.end())
                componentEdges.push_back(k);
        float minX = std::numeric_limits<float>::infinity();
        float maxX = -minX, minY = minX, maxY = -minX;
        for (auto k : componentEdges) {
            const auto center = candidates[k].rect.center;
            minX = std::min(minX, center.x); maxX = std::max(maxX, center.x);
            minY = std::min(minY, center.y); maxY = std::max(maxY, center.y);
        }
        if (vertices.size() >= 6 && vertices.size() <= 18 &&
            maxX - minX <= 180.0f && maxY - minY <= 40.0f) {
            const int n = static_cast<int>(vertices.size());
            const int states = 1 << n;
            std::vector<double> memo(states, std::numeric_limits<double>::quiet_NaN());
            std::vector<std::size_t> choice(states, candidates.size());
            std::vector<std::vector<std::pair<int, std::size_t>>> edges(n);
            for (auto k : componentEdges) {
                if (candidates[k].loss > 11.0) continue;
                int a = std::find(vertices.begin(), vertices.end(), candidates[k].first) - vertices.begin();
                int b = std::find(vertices.begin(), vertices.end(), candidates[k].second) - vertices.begin();
                edges[a].push_back({b, k}); edges[b].push_back({a, k});
            }
            std::function<double(int)> solve = [&](int mask) -> double {
                if (!mask) return 0.0;
                double& best = memo[mask];
                if (best == best) return best;
                int v = __builtin_ctz(static_cast<unsigned>(mask));
                int rest = mask & ~(1 << v);
                best = solve(rest);
                for (auto [w, k] : edges[v]) {
                    if (!(rest & (1 << w))) continue;
                    double score = candidates[k].loss - 20.0 + solve(rest & ~(1 << w));
                    if (score < best) { best = score; choice[mask] = k; }
                }
                return best;
            };
            int mask = states - 1; solve(mask);
            while (mask) {
                int v = __builtin_ctz(static_cast<unsigned>(mask));
                auto k = choice[mask];
                mask &= ~(1 << v);
                if (k != candidates.size()) {
                    int w = std::find(vertices.begin(), vertices.end(),
                                      candidates[k].first == vertices[v] ? candidates[k].second : candidates[k].first)
                            - vertices.begin();
                    mask &= ~(1 << w); selected.push_back(k);
                }
            }
        } else {
            std::vector<bool> used(lightCount, false);
            for (auto k : componentEdges) {
                const auto& c = candidates[k];
                if (used[c.first] || used[c.second]) continue;
                std::size_t first = candidates.size(), second = candidates.size();
                double bestCombined = std::numeric_limits<double>::infinity();
                for (auto a : adjacent[c.first]) {
                    const auto& x = candidates[a];
                    auto otherA = x.first == c.first ? x.second : x.first;
                    if (otherA == c.second || used[otherA]) continue;
                    for (auto b : adjacent[c.second]) {
                        const auto& y = candidates[b];
                        auto otherB = y.first == c.second ? y.second : y.first;
                        if (otherB == c.first || otherB == otherA || used[otherB]) continue;
                        double sum = x.loss + y.loss;
                        if (sum <= c.loss + 3.0 && sum < bestCombined) {
                            bestCombined = sum; first = a; second = b;
                        }
                    }
                }
                if (first != candidates.size()) {
                    for (auto idx : {first, second}) {
                        used[candidates[idx].first] = used[candidates[idx].second] = true;
                        selected.push_back(idx);
                    }
                } else {
                    used[c.first] = used[c.second] = true;
                    selected.push_back(k);
                }
            }
        }
    }
    return selected;
}
