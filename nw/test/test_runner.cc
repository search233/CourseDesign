#include "core/frame_builder.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace ethernet;

// 1. 定义单个测试用例的数据结构
struct TestCase {
    std::string name;
    std::string dest_mac;
    std::string src_mac;
    std::string payload;
    std::string expect_status;
};

// 2. 字符串 Trim 辅助函数
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// 3. 期望状态字符串映射到枚举
ErrorCode string_to_errorcode(const std::string& str) {
    if (str == "Success") return ErrorCode::Success;
    if (str == "InvalidDestMac") return ErrorCode::InvalidDestMac;
    if (str == "InvalidSrcMac") return ErrorCode::InvalidSrcMac;
    if (str == "PayloadTooLarge") return ErrorCode::PayloadTooLarge;
    return ErrorCode::Success; // 默认 fallback
}

// 4. INI 文件解析器
std::vector<TestCase> load_test_cases(const std::string& filepath) {
    std::vector<TestCase> cases;
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "[错误] 无法打开测试文件: " << filepath << "\n";
        return cases;
    }

    std::string line;
    TestCase current_case;
    bool in_section = false;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue; // 忽略空行和注释

        if (line.front() == '[' && line.back() == ']') {
            if (in_section) {
                cases.push_back(current_case); // 保存上一个 Section
            }
            current_case = TestCase{};
            current_case.name = line.substr(1, line.size() - 2);
            in_section = true;
        } else if (in_section) {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));
                
                if (key == "dest_mac") current_case.dest_mac = value;
                else if (key == "src_mac") current_case.src_mac = value;
                else if (key == "payload") current_case.payload = value;
                else if (key == "expect_status") current_case.expect_status = value;
            }
        }
    }
    if (in_section) cases.push_back(current_case); // 压入最后一个

    return cases;
}

// 5. 运行所有测试
int main() {
    std::cout << "======================================\n";
    std::cout << "  IEEE 802.3 核心逻辑自动化测试工具   \n";
    std::cout << "======================================\n\n";

    // 假设 cmake 把 ini 拷贝到了同级目录
    auto test_cases = load_test_cases("test_cases.ini");
    if (test_cases.empty()) return -1;

    int passed = 0;
    int failed = 0;

    for (const auto& tc : test_cases) {
        std::cout << "▶ 运行测试: [" << tc.name << "]\n";
        
        // 执行底层封装
        Result res = builder::encapsulate(tc.dest_mac, tc.src_mac, tc.payload);
        
        // 校验结果
        ErrorCode expected = string_to_errorcode(tc.expect_status);
        bool is_pass = (res.status == expected);

        if (is_pass) {
            std::cout << "  [PASS] 状态符合预期 (" << tc.expect_status << ")\n";
            passed++;
            
            // 针对成功的用例，额外打印一些帧结构验证信息
            if (res.is_ok()) {
                std::cout << "         [验证] 原始长度: " << res.frame.payload_length 
                          << " | Padding: " << res.frame.padding.size() 
                          << " | FCS: 0x" << std::hex << std::uppercase << res.frame.fcs << std::dec << "\n";
            }
        } else {
            std::cout << "  [FAIL] 期望: " << tc.expect_status 
                      << ", 实际得到: " << error_to_string(res.status) << "\n";
            failed++;
        }
        std::cout << "--------------------------------------\n";
    }

    std::cout << "\n=== 测试汇总 ===\n";
    std::cout << "总计: " << test_cases.size() << " | 通过: " << passed << " | 失败: " << failed << "\n";
    
    return failed == 0 ? 0 : -1;
}