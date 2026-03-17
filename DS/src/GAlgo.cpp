#include "GAlgo.h"
#include "Utils.h"

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

bool GAlgo::is_connected(const Graph& graph) {
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

    kruskal kru(n, edges);
    return kru.check();
}

std::vector<std::tuple<City, int>> GAlgo::cal_path_dist(int s, const Graph& graph) {
    Dijkstra dij(graph);
    dij.run(s);
    int n = graph.city_cnt();

    std::vector<std::tuple<City, int>> res;
    for (int i = 0; i < n; ++i) {
        res[i] = std::make_tuple(graph.get_city(i), dij.dist[i]);
    }

    std::sort(res.begin(), res.end(), 
        [&](const std::tuple<City, int>& a, const std::tuple<City, int>& b) {
            return std::get<1>(a) < std::get<1>(b);
        });

    return res;
}

std::vector<Edge> GAlgo::MST(const Graph& graph) {
    int n = graph.city_cnt();
    std::vector<Edge> edges;

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
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
