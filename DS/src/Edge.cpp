#include "Edge.h"
#include <sstream>

Edge::Edge(int id1, int id2, int distance) :
    id1(id1), id2(id2), distance(distance){}

std::string Edge::to_string() {
    std::stringstream ss;

    ss << id1 << " <-------> " << id2 << '\n';

    return ss.str();
}

std::tuple<int, int, int> Edge::get_info() {
    auto edge_info = std::make_tuple(id1, id2, distance);
    return edge_info;
}
