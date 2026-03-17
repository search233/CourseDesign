#include "Graph.h"
#include <tuple>
#include <algorithm>

bool Graph::add_city(const City& city) {
    auto [id, name, x, y, brief] = city.get_info();
    if (mp.count(id)) {
        return false;
    }

    city_list.push_back(city);
    edge_list.push_back(std::vector<Edge>());
    mp[id] = (int)city_list.size() - 1;
    return true;
}

bool Graph::add_edge(const Edge& edge) {
    auto [id1, id2, dis] = edge.get_info();

    if (!mp.count(id1) || !mp.count(id2)) {
        return false;
    }
    int u = mp[id1];
    int v = mp[id2];
    
    edge_list[u].push_back(Edge(u, v, dis));
    edge_list[v].push_back(Edge(v, u, dis));

    return true;
}

bool Graph::remove_edge(int id1, int id2) {
    if (!mp.count(id1) || !mp.count(id2)) {
        return false;
    }
    int u = mp[id1];
    int v = mp[id2];

    edge_list[u].erase(std::remove_if(edge_list[u].begin(), edge_list[u].end(), 
        [&v](const Edge& e) -> bool { 
            return std::get<1>(e.get_info()) == v; 
        }), 
        edge_list[u].end());

    edge_list[v].erase(std::remove_if(edge_list[v].begin(), edge_list[v].end(), 
        [&u](const Edge& e) -> bool { 
            return std::get<1>(e.get_info()) == u; 
        }), 
        edge_list[v].end());

    return true;
}

bool Graph::remove_city(int id) {
    if (!mp.count(id)) {
        return false;
    }

    int p = mp[id];
    for (auto edge : edge_list[p]) {
        auto [u, v, w] = edge.get_info();
        int id1 = std::get<0>(city_list[u].get_info());
        int id2 = std::get<0>(city_list[v].get_info());

        bool f = remove_edge(id1, id2);
        if (!f) return false;
    }

    city_list.erase(city_list.begin() + p);

    return true;
}

const City& Graph::get_city(int inner_id) const {
    static const City empty;
    if (inner_id < 0 || inner_id >= city_cnt()) {
        return empty;
    }

    return city_list[inner_id];
}

const std::vector<Edge>& Graph::get_edges(int inner_id) const {
    static const std::vector<Edge> empty;
    if (inner_id < 0 || inner_id >= city_cnt()) {
        return empty;
    }
    return edge_list[inner_id];
}

int Graph::city_cnt() const {
    return city_list.size();
}

