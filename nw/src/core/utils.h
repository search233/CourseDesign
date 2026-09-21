
#pragma once

#include "frame_type.h"

namespace utils {

// 解析字符串，得到一个6字节mac地址
ethernet::ErrorCode 
    parse_mac 
        (const std::string& mac_str, 
        std::vector<uint8_t>& mac,
        bool is_dest = true);

// 将单个字节转化成大写的16进制串
std::string byte_to_hex(uint8_t byte);

// 将mac地址转换成字符串
std::string 
    mac_to_string (const std::array<uint8_t, 6>& mac);

// ascii 转化为字节串
std::vector<uint8_t> ascii_to_bytes (const std::string& ascii_str);

// 将字节转化成二进制串
std::string byte_to_bit_string(uint8_t byte);
std::string twobyte_to_bitstring (uint16_t byte);


}