#include "Graph.h"
#include <string>

class FileManager {
public :
    static bool load_cities(const std::string& filename, Graph& graph);
    static bool load_edges(const std::string& filename, Graph& graph);

    static bool save_cities(const std::string& filename, const Graph& graph);
    static bool save_edges(const std::string& filename, const Graph& graph);
};
