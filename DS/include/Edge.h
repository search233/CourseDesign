#ifndef EDGE_H
#define EDGE_H

#include <string>
#include <tuple>

class Edge {

private :
    int id1;
    int id2;

    int distance;

public :
    Edge(int id1, int id2, int distance);

    std::string to_string() const;

    std::tuple<int, int, int> get_info() const;
};

/*
    Edge 包含
        存在路径连接的两个城市的编号
            用两个 int 类型变量存储
        
        存在路径连接的两个城市的距离
            用 int 类型存储

*/

#endif // DS_EDGE_H
