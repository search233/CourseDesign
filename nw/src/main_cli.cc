#include "core/frame_builder.h"
#include <iostream>
#include <iomanip>
#include <string>

using namespace ethernet;

void print_hex_dump(const std::vector<uint8_t>& bytes) {

    for (size_t i = 0; i < bytes.size(); ++i) {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(bytes[i]) << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << "\n";
        } else if ((i + 1) % 8 == 0) {
            std::cout << "  "; // 每 8 字节额外加个空格，方便人眼对齐
        }
    }
    std::cout << std::dec << "\n";
}

int main() {
    std::cout << "======================================\n"
              << "   IEEE 802.3 Frame Builder (CLI)     \n"
              << "======================================\n\n";

    // 1. 模拟界面输入的数据
    std::string dst_mac = "FF:FF:FF:FF:FF:FF";  // 广播地址
    std::string src_mac = "00:1A:2B:3C:4D:5E";  // 随机合法 MAC
    std::string payload = "Hello 802.3";        // 长度为 11，不足 46，将触发 Padding

    std::cout << "[Input] Dest MAC: " << dst_mac << "\n";
    std::cout << "[Input] Src MAC:  " << src_mac << "\n";
    std::cout << "[Input] Payload:  " << payload << " (Ascii)\n\n";

    // 2. 调用顶层封装门面
    Result res = builder::encapsulate(dst_mac, src_mac, payload);

    // 3. 错误防御测试
    if (!res.is_ok()) {
        std::cerr << "[Error] 封装失败! 错误信息: " << res.error_message << "\n";
        return -1;
    }

    // 4. 封装成功，打印统计信息
    std::cout << "--- 帧结构统计 ---\n";
    std::cout << "有效载荷长度: " << res.frame.payload_length << " 字节\n";
    std::cout << "Padding 填充: " << res.frame.padding.size() << " 字节\n";
    std::cout << "CRC-8 校验和: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') 
              << res.frame.fcs << std::dec << "\n";
    std::cout << "物理帧总长度: " << res.frame.get_size() << " 字节\n\n";

    // 5. 打印完整的序列化十六进制结果 (最终在物理线上跑的数据)
    std::cout << "--- 完整以太网帧流 (Hex Dump) ---\n";
    print_hex_dump(res.frame.serialize());
    std::cout << "\n";

    // 6. 验证 CRC 引擎的追踪功能是否工作正常
    std::cout << "--- CRC 运算追踪快照 (取前 5 步展示) ---\n";
    std::cout << "总运算步数: " << res.crc_snaps.size() << " 步\n";
    
    // 只打印前 5 步防止刷屏
    size_t display_count = std::min<size_t>(5, res.crc_snaps.size());
    for (size_t i = 0; i < display_count; ++i) {
        const auto& snap = res.crc_snaps[i];
        std::cout << "Step " << std::setw(3) << snap.index 
                  << " | Window: " << snap.current_bits
                  << " | XOR: " << (snap.is_xor ? "Yes" : "No ")
                  << " | Remainder: " << snap.remainder
                  << " | Next In: " << (snap.next_bit == '\0' ? '-' : snap.next_bit) 
                  << "\n";
    }
    
    std::cout << "...\n[校验计算引擎与追踪矩阵已就绪]\n";

    return 0;
}