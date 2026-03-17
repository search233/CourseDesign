#include <string>
#include <vector>
#include "City.h"

namespace Utils {

    int cal_Dis(const City& a, const City& b);

    std::vector<std::string> split(const std::string& s, char delim);

    bool isInteger(const std::string& s);

    std::string trimCR(const std::string& s);

}
