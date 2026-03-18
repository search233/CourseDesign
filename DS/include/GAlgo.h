#ifndef GALGO_H
#define GALGO_H

#include "Graph.h"
#include <algorithm>
#include <queue>
#include <array>


class GAlgo {
private :
    struct DSU {
        
        int n;
        std::vector<int> fa, sz;
    
        void init(int n);
    
        int find(int x);
    
        bool same(int u, int v);
    
        void merge(int u, int v);
    
        int size(int x);
    };

    struct Dijkstra {
    
        int n;
        std::vector<std::vector<Edge>> edges;
        std::vector<int> dist; 
    
        Dijkstra(const Graph& graph);
    
        void run(int s);
    };

    struct kruskal {
        int n;
        DSU dsu;
        std::vector<Edge> e;

        kruskal(int cnt, const std::vector<Edge>& edges);

        std::vector<Edge> run();

        bool check();
    };

    static std::vector<Edge> grab_edges(const Graph& graph);
public :

    // 返回完全联通需要加上的和最小的边
    // 完全联通返回空
    static std::vector<Edge> is_connected(const Graph& graph);
 
    // 按照距离从小到大返回一个城市列表
    static std::vector<std::tuple<City, int>> cal_path_dist(int s, const Graph& graph);


    // 问题7 (旅行商问题)
    /*
        使用近似算法解决，城市数量 n <= 200
        1. 先用 Floyd-Warshall 计算所有点对最短距离
        2. 最近邻算法：从起点出发，每次选择最近的未访问城市
        3. 适用于路径问题（不回到起点）和回路问题（回到起点）
    */
    struct TSP {
        int n;
        std::vector<std::vector<int>> dist;  // 所有点对最短距离
        std::vector<std::vector<int>> next;  // 路径记录，用于还原完整路径
        
        TSP(const Graph& graph);
        
        // Floyd-Warshall 计算所有点对最短距离
        void floyd_warshall(const Graph& graph);
        
        // 最近邻算法求解 TSP 路径（不回到起点）
        // 返回 {最短距离, 访问顺序}
        std::pair<int, std::vector<int>> nearest_neighbor_path(int start);
        
        // 最近邻算法求解 TSP 回路（回到起点）
        // 返回 {最短距离, 访问顺序}
        std::pair<int, std::vector<int>> nearest_neighbor_cycle(int start);
        
        // 获取两个城市之间的最短路径上的下一个城市（用于还原完整路径）
        int get_next_city(int u, int v) const;
    };
    
    // 从某一城市出发经过所有城市（城市可重复）且距离最短的路线（不回到起点）
    static std::pair<int, std::vector<int>> tsp_shortest_path(int start, const Graph& graph);
    
    // 从某一城市出发经过所有城市（城市可重复）并回到源点的最短路线
    static std::pair<int, std::vector<int>> tsp_shortest_cycle(int start, const Graph& graph);

    // 最小树问题  
    /*
        数据量较小考虑暴力解决
        尝试建立完全图，用 kruskal 解决
        城市数量为 n 
        建图 时间复杂度 O(n^2)
        边数 m = n * (n - 1)
        kruskal 复杂度 O(m long m)
        n = 2e^2 的情况下该方法可以接受
    */
    static std::vector<Edge> MST(const Graph& graph);

};

#endif