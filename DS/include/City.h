#ifndef CITY_H
#define CITY_H

#include <string>
#include <tuple>

class City { 
private : 
    // 编号 
    int id; 
    // 名称 
    std::string name; 
    // 坐标 
    int x, y; 
    // 简述 
    std::string brief; 
public : 
    City(); 
    City(int id, std::string& name, int x, int y, std::string& brief); 
    
    void set_info(int id, std::string&& name, int x, int y, std::string&& brief); 
    
    std::string to_string() const; 

    std::tuple<int, std::string, int, int, std::string> get_info() const;

}; 
/* city 包含： 
    编号 
        以 int 类型存储 
    名称 
        用 string 存储 
    横纵坐标 
        用两个 int 类型变量存储 x, y 坐标 
    简介 用 string 存储 
*/
#endif
