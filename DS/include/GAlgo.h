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


    // TODO 问题7 (旅行商问题)

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