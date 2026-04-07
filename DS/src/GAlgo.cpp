#include "GAlgo.h"
#include "Utils.h"
#include <set>

void GAlgo::DSU::init(int n) {
    this->n = n;
    fa.resize(n + 1);
    sz.resize(n + 1, 1);
    for (int i = 0; i <= n; i++) {
        fa[i] = i;
    }
}

int GAlgo::DSU::find(int x) {
    if (fa[x] != x)
        fa[x] = find(fa[x]);
    return fa[x];
}

bool GAlgo::DSU::same(int u, int v) {
    return find(u) == find(v);
}

void GAlgo::DSU::merge(int u, int v) {
    u = find(u);
    v = find(v);
    if (u == v) return;

    if (sz[u] < sz[v]) std::swap(u, v);
    
    fa[v] = u;
    sz[u] += sz[v];
}

int GAlgo::DSU::size(int x) {
    return sz[find(x)];
}

GAlgo::Dijkstra::Dijkstra(const Graph& graph) {
    n = graph.city_cnt();
    dist.resize(n);
    edges.resize(n);
    for (int i = 0; i < n; ++i) {
        edges[i] = graph.get_edges(i);
    }
}

void GAlgo::Dijkstra::run(int s) {
    std::priority_queue<std::pair<int,int>, 
                        std::vector<std::pair<int,int>>, 
                        std::greater<std::pair<int,int>>> pq;
                        
    std::fill(dist.begin(), dist.end(), 1e9);

    dist[s] = 0;
    pq.push({0, s});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue;  // 懒删除

        for (auto e : edges[u]) {
            auto [u, v, w] = e.get_info();

            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
}

GAlgo::kruskal::kruskal(int cnt, const std::vector<Edge>& edges) {
    n = cnt;
    dsu.init(n);
    e.clear();
    e = edges;

    std::sort(e.begin(), e.end(),
        [](const Edge& a, const Edge& b) {
            int wa = std::get<2>(a.get_info());
            int wb = std::get<2>(b.get_info());
            return wa < wb;
        });
}

std::vector<Edge> GAlgo::kruskal::run() {
    std::vector<Edge> res;
    for (auto edge : e) {
        auto [u, v, w] = edge.get_info();
        if (dsu.same(u, v)) continue;
        dsu.merge(u, v);
        res.push_back(edge);
    }

    return res;
}

bool GAlgo::kruskal::check() {
    run();
    return dsu.size(0) == dsu.n;
}

std::vector<Edge> GAlgo::grab_edges(const Graph& graph) {
    int n = graph.city_cnt();
    std::vector<Edge> edges;

    for (int i = 0; i < n; ++i) {
        auto tmp = graph.get_edges(i);
        for (auto j : tmp) {
            auto [u, v, w] = j.get_info();
            if (u > v) continue;

            edges.push_back(j);
        }
    }

    return edges;
}
 

std::vector<Edge> GAlgo::is_connected(const Graph& graph) {
    static const std::vector<Edge> Empty;
    int n = graph.city_cnt(); 
    DSU dsu;

    dsu.init(n);
    std::vector<Edge> edges = grab_edges(graph);
    for (auto e : edges) {
        auto [u, v, w] = e.get_info();
        dsu.merge(u, v);
    }

    std::set<int> st;
    for (int i = 0; i < n; ++i) {
        st.insert(dsu.find(i));
    }

    if (st.size() == 1) {
        return Empty;
    }

    edges.clear();

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (dsu.same(i, j)) continue;
            City a =  graph.get_city(i);
            City b =  graph.get_city(j);
            Edge e(i, j, Utils::cal_Dis(a, b));
            edges.push_back(e);
        }
    }

    sort(edges.begin(), edges.end(),
        [](const Edge& a, const Edge& b) {
            return std::get<2>(a.get_info()) < std::get<2>(b.get_info());
        });

    kruskal kru(n, edges);
    return kru.run();
}


std::vector<std::tuple<City, int>> 
    GAlgo::cal_path_dist(int s, const Graph& graph) {

    s = graph.get_innerid(s);
    Dijkstra dij(graph);
    dij.run(s);
    int n = graph.city_cnt();

    std::vector<std::tuple<City, int>> res;
    res.reserve(n);
    for (int i = 0; i < n; ++i) {
        res.emplace_back(graph.get_city(i), dij.dist[i]);
    }

    std::sort(res.begin(), res.end(), 
        [&](const std::tuple<City, int>& a, 
            const std::tuple<City, int>& b) {                
            return std::get<1>(a) < std::get<1>(b);
        });

    return res;
}

std::vector<Edge> GAlgo::MST(const Graph& graph) {
    int n = graph.city_cnt();
    std::vector<Edge> edges;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            City a = graph.get_city(i);
            City b = graph.get_city(j);
            int dis = Utils::cal_Dis(a, b);
            Edge e(i, j, dis);
            edges.push_back(e);
        }    
    }

    kruskal kru(n, edges);
    return kru.run();
}

// ==================== TSP 实现 ====================

GAlgo::TSP::TSP(const Graph& graph) {
    n = graph.city_cnt();
    floyd_warshall(graph);
}

void GAlgo::TSP::floyd_warshall(const Graph& graph) {
    const int INF = 1e9;
    
    // 初始化距离矩阵和路径矩阵
    dist.assign(n, std::vector<int>(n, INF));
    next.assign(n, std::vector<int>(n, -1));
    
    for (int i = 0; i < n; ++i) {
        dist[i][i] = 0;
        next[i][i] = i;
    }
    
    // 从图中读取直接相连的边
    for (int u = 0; u < n; ++u) {
        auto edges = graph.get_edges(u);
        for (const auto& e : edges) {
            auto [from, v, w] = e.get_info();
            if (w < dist[u][v]) {
                dist[u][v] = w;
                next[u][v] = v;
            }
        }
    }
    
    // Floyd-Warshall 算法
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
    }
}

int GAlgo::TSP::get_next_city(int u, int v) const {
    return next[u][v];
}

std::pair<int, std::vector<int>> GAlgo::TSP::nearest_neighbor_path(int start) {
    const int INF = 1e9;
    std::vector<bool> visited(n, false);
    std::vector<int> path;
    int total_dist = 0;
    int current = start;
    int visited_count = 0;
    
    visited[current] = true;
    path.push_back(current);
    visited_count++;
    
    while (visited_count < n) {
        int next_city = -1;
        int min_dist = INF;
        
        // 找到最近的未访问城市
        for (int i = 0; i < n; ++i) {
            if (!visited[i] && dist[current][i] < min_dist) {
                min_dist = dist[current][i];
                next_city = i;
            }
        }
        
        if (next_city == -1) {
            // 无法到达更多城市（图不连通）
            break;
        }
        
        total_dist += min_dist;
        current = next_city;
        visited[current] = true;
        path.push_back(current);
        visited_count++;
    }
    
    return {total_dist, path};
}

std::pair<int, std::vector<int>> GAlgo::TSP::nearest_neighbor_cycle(int start) {
    const int INF = 1e9;
    std::vector<bool> visited(n, false);
    std::vector<int> path;
    int total_dist = 0;
    int current = start;
    int visited_count = 0;
    
    visited[current] = true;
    path.push_back(current);
    visited_count++;
    
    while (visited_count < n) {
        int next_city = -1;
        int min_dist = INF;
        
        // 找到最近的未访问城市
        for (int i = 0; i < n; ++i) {
            if (!visited[i] && dist[current][i] < min_dist) {
                min_dist = dist[current][i];
                next_city = i;
            }
        }
        
        if (next_city == -1) {
            // 无法到达更多城市（图不连通）
            break;
        }
        
        total_dist += min_dist;
        current = next_city;
        visited[current] = true;
        path.push_back(current);
        visited_count++;
    }
    
    // 回到起点形成回路
    total_dist += dist[current][start];
    path.push_back(start);
    
    return {total_dist, path};
}

std::pair<int, std::vector<int>> GAlgo::tsp_shortest_path(int s, const Graph& graph) {
    TSP tsp(graph);
    s = graph.get_innerid(s);
    return tsp.nearest_neighbor_path(s);
}

std::pair<int, std::vector<int>> GAlgo::tsp_shortest_cycle(int s, const Graph& graph) {
    TSP tsp(graph);
    s = graph.get_innerid(s);
    return tsp.nearest_neighbor_cycle(s);
}
