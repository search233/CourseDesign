#include "utils.h"
#include "crc_engine.h"

namespace ethernet::crc {

uint8_t calculate_crc8(
    const std::vector<uint8_t>& data, 
    std::vector<CrcSnapshot>& trace,
    uint16_t poly) {

    trace.clear();
    if (data.empty()) {
        return 0;
    }

    // 1. 数据展开为二进制串
    std::string bits = utils::bytes_to_bit_string(data);
    
    // 2. 尾部追加 8 个 '0'
    bits += std::string(8, '0');

    // 3. 提取 9 位有效多项式二进制串 (如 0x107 -> "100000111")
    std::string poly_bits = utils::byte_to_bit_string(poly).substr(7);

    // 4. 初始化 9 位的滑动计算窗口
    std::string cur_window = bits.substr(0, 9);
    
    // 需执行异或判断的总次数
    size_t total_steps = bits.length() - 8;
    trace.reserve(total_steps);

    // 5. 开始模 2 长除法循环
    for (size_t i = 0; i < total_steps; ++i) {
        CrcSnapshot snap;
        snap.index = i;
        snap.current_window = cur_window;
        
        // 若首位为 1，则本轮需要进行异或运算
        snap.is_xor = (cur_window[0] == '1');
        
        if (snap.is_xor) {
            for (size_t j = 0; j < 9; ++j) {
                cur_window[j] = (cur_window[j] == poly_bits[j]) ? '0' : '1';
            }
        }
        
        // 记录异或或仅仅直移后的中间状态
        snap.remainder = cur_window;
        
        // 判读是否还有下一位要滑入
        if (i + 9 < bits.length()) {
            snap.next_bit = bits[i + 9];
            // 寄存器左移 1 位，读入新比特
            cur_window = cur_window.substr(1) + snap.next_bit;
        } else {
            snap.next_bit = '\0'; // 运算完毕
        }
        
        trace.push_back(snap);
    }

    // 6. 最终的余数截取最后 8 位
    std::string final_remainder = trace.back().remainder.substr(1);
    
    return static_cast<uint8_t>(std::stoi(final_remainder, nullptr, 2));
}

}
