#ifndef GRAPH_H
#define GRAPH_H

#include "City.h"
#include "Edge.h"
#include "Utils.h"
#include <vector>
#include <unordered_map>

class Graph {
private :
    std::unordered_map<int, int> mp;
    std::vector<City> city_list;
    std::vector<std::vector<Edge>> edge_list;

public :
    bool add_edge(int id1, int id2);
    bool add_city(const City& city);

    bool remove_edge(int id1, int id2);
    bool remove_city(int id);

    const City& get_city(int inner_id) const;
    const std::vector<Edge>& get_edges(int inner_id) const;

    int city_cnt() const;
};
/*
    Graph 包含
        存储城市信息的表
            vector<City>
        存储图的邻接表
            vector<vector<Edge>>
            邻接表中的编号用内部编号
        映射城市编号与内部编号的哈希表
            unordered_map<int, int> 
            城市编号 -----> 内部编号
*/
#endif;
