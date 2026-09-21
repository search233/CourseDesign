#include "core/frame_builder.h"
#include "core/frame_type.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// 测试用例数据结构
struct TestCase {
    std::string id;
    std::string description;
    std::string dest_mac;
    std::string src_mac;
    std::string payload;
    std::string expect_status;
};

// 辅助函数：去除字符串首尾空白字符
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// 简易 INI 解析器（从重定向后的标准输入流中解析）
std::vector<TestCase> parse_test_cases_from_cin() {
    std::vector<TestCase> cases;
    TestCase current_case;
    std::string line;

    while (std::getline(std::cin, line)) {
        line = trim(line);
        // 跳过空行或注释行
        if (line.empty() || line.front() == ';') continue;

        if (line.front() == '[' && line.back() == ']') {
            if (!current_case.id.empty()) {
                cases.push_back(current_case);
                current_case = TestCase{};
            }
            current_case.id = line.substr(1, line.length() - 2);
        } else {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string key = trim(line.substr(0, eq));
                std::string val = trim(line.substr(eq + 1));
                
                if (key == "description") current_case.description = val;
                else if (key == "dest_mac") current_case.dest_mac = val;
                else if (key == "src_mac") current_case.src_mac = val;
                else if (key == "payload") current_case.payload = val;
                else if (key == "expect_status") current_case.expect_status = val;
            }
        }
    }
    if (!current_case.id.empty()) {
        cases.push_back(current_case);
    }
    return cases;
}

int main() {
    // 1. 打开 INI 文件并重定向标准输入 (满足 "main中重定向输入打开ini文件作为测试程序的输入")
    std::ifstream ini_file("test_cases.ini");
    if (!ini_file.is_open()) {
        std::cerr << "[错误] 无法打开测试配置文件 test_cases.ini，请检查路径！\n";
        return 1;
    }
    std::cin.rdbuf(ini_file.rdbuf());

    // 2. 解析测试用例
    std::vector<TestCase> test_cases = parse_test_cases_from_cin();
    
    std::cout << "======================================\n";
    std::cout << "   IEEE 802.3 Automated Test Runner   \n";
    std::cout << "======================================\n";
    std::cout << "成功加载测试用例数: " << test_cases.size() << "\n\n";

    int passed_count = 0;

    // 3. 循环执行测试
    for (const auto& tc : test_cases) {
        std::cout << "--------------------------------------\n";
        std::cout << "执行用例 [" << tc.id << "]: " << tc.description << "\n";
        std::cout << "  - 输入 Dest MAC : " << tc.dest_mac << "\n";
        std::cout << "  - 输入 Src MAC  : " << tc.src_mac << "\n";
        std::cout << "  - 输入 Payload 长度  : " << tc.payload.size() << "\n";
        std::cout << "  - 预期结果状态  : " << tc.expect_status << "\n";

        // 调用底层核心封装
        auto res = ethernet::builder::encapsulate(tc.dest_mac, tc.src_mac, tc.payload);

        // std::cout << "数据长度：  " << 
        
        // 映射实际运行状态信息
        std::string actual_status = res.is_ok() ? "封装成功" : res.error_message;
        std::cout << "  - 实际运行状态  : " << actual_status << "\n";

        // 4. 断言判定
        if (actual_status == tc.expect_status) {
            std::cout << "  => 【测试结果: 通过 (PASS)】\n";
            passed_count++;
        } else {
            std::cout << "  => 【测试结果: 失败 (FAIL)】状态不匹配！\n";
        }
    }

    std::cout << "======================================\n";
    std::cout << "测试汇总: 通过 " << passed_count << " / " << test_cases.size() << "\n";
    std::cout << "======================================\n";

    // 返回退出码（全部通过返回 0，有失败返回 1）
    return (static_cast<size_t>(passed_count) == test_cases.size()) ? 0 : 1;
}