#include "Utils.h"
#include <cmath>

int Utils::cal_Dis(const City& a, const City& b) {
    auto [a_id, a_name, a_x, a_y, a_brief] = a.get_info();
    auto [b_id, b_nbme, b_x, b_y, b_brief] = b.get_info();

    int dx = a_x - b_x;
    int dy = a_y - b_y;

    return  round(sqrt(dx * dx + dy * dy));
}

std::vector<std::string> Utils::split(const std::string& s, char delim) {
    std::vector<std::string> res;
    std::string cur = "";

    for (auto c : s) {
        if (c == delim && !cur.empty()) {
            res.push_back(cur);
            cur = "";
        }
        else if (c != delim){
            cur.push_back(c);
        }
    }

    return res;
}

