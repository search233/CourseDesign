#include <string>
#include <tuple>

class Edge {

private :
    int id1;
    int id2;

    int distance;

public :
    Edge(int id1, int id2, int distance) :
        id1(id1), id2(id2), distance(distance){}

    std::string to_string();

    std::tuple<int, int, int> get_info();
};
