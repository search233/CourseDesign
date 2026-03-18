#ifndef MENU_H
#define MENU_H

#include "Graph.h"
#include "Utils.h"
#include "GAlgo.h"

class Menu {
private :
    int run_tag;
    Graph graph;

    void show_menu();
    void choose_op(int op);
    void shut_down();


    void add_edge();
    void add_city();
    void remove_city();
    void remove_edge();

    void show_cites();
    void show_graph();

    // 问题5
    void is_connected();
    // 问题6
    void show_dis();
    // TODO 问题7

    // 问题8
    void show_kru_path();

public :
    Menu();

    void run();
};

#endif
