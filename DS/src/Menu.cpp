#include "Menu.h"
#include "FileManager.h"
#include <iostream>

const std::string city_file = "/home/yangz/Projects/CourseDesign/DS/data/cities.txt";
const std::string edge_file = "/home/yangz/Projects/CourseDesign/DS/data/edges.txt";

Menu::Menu() {
    run_tag = 1;
    FileManager::load_cities(city_file, graph);
    FileManager::load_edges(edge_file, graph);
}

void Menu::run() {
    while (run_tag) {
        show_menu();

        int op;
        std::cout << "输入操作：  ";
        std::cin >> op;

        choose_op(op);
    } 
}

void Menu::shut_down() {
    run_tag = 0;
    FileManager::save_cities(city_file, graph);
    FileManager::save_edges(edge_file, graph);
}

void Menu::show_menu() {
    std::cout << "\n===== 通信网络系统 =====\n";
    std::cout << "1. 添加城市\n";
    std::cout << "2. 添加通信线路\n";
    std::cout << "3. 删除城市\n";
    std::cout << "4. 删除通信线路\n";
    std::cout << "5. 展示所有城市\n";
    std::cout << "6. 展示邻接表\n";
    std::cout << "7. 判断连通性\n";
    std::cout << "8. 最短路径查询\n";
    std::cout << "9. 构建最小生成树\n";
    std::cout << "10. TSP 路径\n";
    std::cout << "0. 退出\n";
}

void Menu::choose_op(int op) {
    switch (op) {
        case 1: add_city(); break;
        case 2: add_edge(); break;
        case 3: remove_city(); break;
        case 4: remove_edge(); break;
        case 5: show_cites(); break;
        case 6: show_graph(); break;
        case 7: is_connected(); break;
        case 8: show_dis(); break;
        case 9: show_kru_path(); break;
        case 10: show_tsp_path(); break;
        case 0: shut_down(); break;
        default:
            std::cout << "无效选项" << std::endl;
    }
}

void Menu::add_city() {
    std::string name, brief;
    int x, y;

    int id;
    std::cout << "输入城市编号: ";
    std::cin >> id;

    std::cout << "输入城市名称: ";
    std::cin >> name;

    std::cout << "输入坐标 x y: ";
    std::cin >> x >> y;

    std::cin.ignore();
    std::cout << "输入描述: ";
    getline(std::cin, brief);

    if (graph.add_city({id, name, x, y, brief})) {
        std::cout << "添加成功, ID = " << id << std::endl;
    }
    else {
        std::cout << "添加失败， ID = " << id <<  "的城市已存在，是否覆盖？[Y/n]\n";
        char choice;
        std::cin >> choice;
        if (choice == 'Y' || choice == 'y') {
                City newCity(id, name, x, y, brief);
                graph.add_city(newCity);
                std::cout << "覆盖成功, ID = " << id << std::endl;
        } else {
            std::cout << "放弃覆盖" << std::endl;
        }
    }    
}

void Menu::add_edge() {
    int u, v;
    std::cout << "输入两个城市ID: ";
    std::cin >> u >> v;
    
    if (graph.add_edge(u, v)) {
        std::cout << "添加成功\n";
    } else {
        std::cout << "添加失败\n";
    }
}

void Menu::remove_city() {
    int id;
    std::cout << "输入要删除的城市ID: ";
    std::cin >> id;

    if (graph.remove_city(id)) {
        std::cout << "删除成功\n";
    } else {
        std::cout << "删除失败，城市不存在\n";
    }
}

void Menu::remove_edge() {
    int u, v;
    std::cout << "输入要删除线路的两个城市ID: ";
    std::cin >> u >> v;

    if (graph.remove_edge(u, v)) {
        std::cout << "删除成功\n";
    } else {
        std::cout << "删除失败，城市不存在或线路不存在\n";
    }
}

void Menu::show_cites() {
    int n = graph.city_cnt();
    for (int i = 0; i < n; ++i) {
        auto [id, name, x, y, brief] = 
            graph.get_city(i).get_info();

        std::cout << "\n-------------------";
        std::cout << "\n编号 = " << id;
        std::cout << "\n名称 = " << name;
        std::cout << "\n坐标 = (" <<x << ' ' << y;
        std::cout << ")\n简述 = " << brief;
    }
    std::cout << "\n-------------------\n" << std::endl;
}

void Menu::show_graph() {
    int n = graph.city_cnt();

    for (int i = 0; i < n; ++i) {
        City u_city = graph.get_city(i);
        std::cout <<  std::get<0>(u_city.get_info()) 
                  << " ------> ";

        auto vec = graph.get_edges(i);
        int sz = vec.size() - 1;
        for (int j = 0; j <= sz; ++j) {
            auto [u, v, w] = vec[j].get_info();
            auto [id, name, x, y, brief] = 
                graph.get_city(v).get_info();
            std::cout << ' ' << id;
        }
        std::cout << '\n';
    }
}

void Menu::is_connected() {
    std::vector<Edge> vec = GAlgo::is_connected(graph);
    if (vec.empty()) {
        std::cout << "当前所有城市完全连通" << std::endl;
    }
    else {
        std::cout << "当前所有城市不完全连通\n需要加入下面路径:\n";
        for (auto e : vec) {
            auto [u, v, w] = e.get_info();
            int uid = std::get<0>(graph.get_city(u).get_info());
            int vid = std::get<0>(graph.get_city(v).get_info());

            std::cout << uid << " <----> " << vid << '\n';
        }
    }
}

void Menu::show_dis() {
    std::cout << "输入出发点：";
    int s; std::cin >> s;

    std::vector<std::tuple<City, int>> list = 
        GAlgo::cal_path_dist(s, graph);

    for (auto i : list) {
        City city = std::get<0>(i);
        int dis = std::get<1>(i);
        int id = std::get<0>(city.get_info());
        if (dis == 1e9) {
            std::cout << "与城市 " << id  << " 不连通\n";
        }
        else {
            std::cout << "距离城市 " << id << ' ' << dis << '\n';
        }
        
    }
}

void Menu::show_kru_path() {
    std::vector<Edge> vec = GAlgo::MST(graph);
    for (auto e : vec) {
        auto [u, v, w] = e.get_info();
        int uid = std::get<0>(graph.get_city(u).get_info());
        int vid = std::get<0>(graph.get_city(v).get_info());

        std::cout << uid << " <----> " << vid << '\n';
    }
}

void Menu::show_tsp_path() {
    std::cout << "输入起始城市：";
    int s;
    std::cin >> s;

    auto [dist, path] = GAlgo::tsp_shortest_path(s, graph);
    std::cout << "最短路径距离 = " << dist << std::endl;
    std::cout << "访问顺序：";
    for (int idx : path) {
        int cid = std::get<0>(graph.get_city(idx).get_info());
        std::cout << cid << ' ';
    }
    std::cout << std::endl;
}
