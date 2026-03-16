#include "City.h" 
#include <sstream> 

// 构造 
City::City() : id(-1), name(""), x(0), y(0), brief(""){} 
City::City(int id, std::string&& name, int x, int y, std::string&& brief) : 
    id(id), name(name), x(x), y(y), brief(brief) {} 
    
// 修改数据 
void City::set_info(int id, std::string&& name, int x, int y, std::string&& brief) { 
    this -> id = id; 
    this -> name = name; 
    this -> x = x; 
    this -> y = y; 
    this -> brief = brief; 
} 

// 把信息转换为字符串便于打印 
std::string City::to_string() {     
    std::stringstream ss; 

    ss << "编号 : " << id << "\n" 
    << "名称 : " << name << "\n" 
    << "坐标 : (" << x << ',' << y << ")\n" 
    << "简述 : " << brief << "\n"; 
    
    return ss.str(); 
}