#include "FileManager.h"
#include "Utils.h"
#include <fstream>
#include <string>

bool FileManager::load_cities(const std::string& filename, Graph& graph) {
    std::ifstream ifs(filename);

    std::string line;
    while (getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        auto info = Utils::split(line, '|');
        if (info.empty()) continue;

        int id = stoi(info[1]);
        std::string name = info[2];
        int x = stoi(info[3]);
        int y = stoi(info[4]);
        std::string brief = info[5];
        City city(id, name, x, y, brief);

        graph.add_city(city);
    }
    return true;
}

bool FileManager::load_edges(const std::string& filename, Graph& graph) {
    std::ifstream ifs(filename);
    std::string line;
    while (getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        auto info = Utils::split(line, '|');
        if (info.empty()) continue;

        int id1 = stoi(info[1]);
        int id2 = stoi(info[2]);

        City a = graph.get_city(id1);
        City b = graph.get_city(id2);
        // TODO 检查 city a/b 是否存在

        int dis = Utils::cal_Dis(a, b);
        Edge edge(id1, id2, dis);
        graph.add_edge(edge);
    }
    return true;
}

bool FileManager::save_cities(const std::string& filename, const Graph& graph) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        return false;
    }

    int cnt = graph.city_cnt();
    for (int i = 0; i < cnt; ++i) {
        auto city = graph.get_city(i);
        auto [id, name, x, y, brief] = city.get_info();
        ofs << "|" << id
            << "|" << name
            << "|" << x
            << "|" << y
            << "|" << brief << "|\n";
    }
    return true;
}

bool FileManager::save_edges(const std::string& filename, const Graph& graph) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        return false;
    }
        
    int cnt = graph.city_cnt();
    for (int i = 0; i < cnt; ++i) {
        auto edges = graph.get_edges(i);
        for (auto e : edges) {
            auto [u, v, w] = e.get_info();

            if (u > v) continue;
            // 读取的时候建双边 存储的时候去重

            ofs << "|" << u << "|" << v << "|\n";
        }

    }
    return true;
}
