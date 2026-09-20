#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <string>


namespace ethernet {

// 1. 错误码
enum class ErrorCode {
    Success = 0,
    InvalidDestMac,
    InvalidSrcMac,
    PayloadTooLarge
};
    
// 将错误码转化为可读的字符串
std::string error_to_string(ErrorCode);

// 2. 帧字段分段标识
enum class FrameSegmentType {
    Preamble,   // 前导码 (7B)
    SFD,        // 帧首定界符 (1B)
    DestMac,    // 目的 MAC (6B)
    SrcMac,     // 源 MAC (6B)
    Length,     // 长度字段 (2B)
    Payload,    // 数据字段 (0-1500B)
    Padding,    // 补齐填充 (0-46B)
    FCS         // 校验字段 (4B)
};


// 单个字段的区间位置标记
struct FieldSpan {
    FrameSegmentType type;
    size_t offset;
    size_t length;
};


// crc过程中每一步的快照
struct CrcSnapshot {
    // 步数编号
    size_t index;

    // 当前处理的9位数字
    std::string current_bits;

    // 当前步骤的结果
    bool is_xor;

    // 当前余数
    std::string remainder;

    // 下一位参与计算的数字
    char next_bit;
};



struct EthernetFrame {
    
    // 前导码 (7B)
    std::array<uint8_t, 7> preamble 
        = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};

    // 帧首定界符 (1B)
    uint8_t sfd = 0xAB;

    // 目的 MAC (6B)
    std::array<uint8_t, 6> destmac = {};
    // 源 MAC (6B)
    std::array<uint8_t, 6> srcmac = {};

    // 长度字段 (2B)
    uint16_t payload_length = 0;

    // 数据字段
    std::vector<uint8_t> payload;
    // 补齐填充
    std::vector<uint8_t> padding;

    // 校验字段 (4B)
    uint32_t fcs = 0;

    // 获取总长
    size_t get_size() const;

    // 获取完整帧的字节流序列
    std::vector<uint8_t> serialize() const;

    // 获取参与 CRC 校验的部分
    std::vector<uint8_t> get_crc_payload() const;

    // 获取各字段在 serialize() 输出中的切片位置索引
    std::vector<FieldSpan> get_field_spans() const;

    
};



struct Result {
    ErrorCode status = ErrorCode::Success;
    std::string error_message;
    EthernetFrame frame;
    std::vector<CrcSnapshot> crc_snaps;

    bool is_ok() const { 
        return status == ErrorCode::Success; 
    }
};

}

