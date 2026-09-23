#include "utils.h"
#include <cassert>
#include <sstream>
#include <iomanip>

bool utils::parse_mac (
    std::string_view mac_str,
    std::array<uint8_t, 6>& mac) {

    auto hex_to_int = [&](char c) -> int {
        int res = -1;

        if (c >= '0' && c <= '9') res = c - '0';
        else if (c >= 'a' && c <= 'f') res = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') res = c - 'A' + 10;

        return res;
    };

    if (mac_str.length() != 17) return false;

    char del = 0;

    for (int i = 0; i < 6; ++i) {

        int idx = i * 3;
        int hi = hex_to_int(mac_str[idx]);
        int lo = hex_to_int(mac_str[idx + 1]);

        if (hi == -1 || lo == -1) return false;

        mac[i] = static_cast<uint8_t>((hi << 4) | lo);
        
        if (i < 5) {
            char ch = mac_str[idx + 2];

            if (ch != ':' && ch != '-') return false;

            if (del == 0) del = ch;

            if (del && del != ch) return false;
        }
    }

    return true;
}

std::string utils::byte_to_hex(uint8_t byte) {
    static constexpr char hex_chars[] = "0123456789ABCDEF";
    return std::string{hex_chars[(byte >> 4) & 0x0F], hex_chars[byte & 0x0F]};
}


std::string utils::mac_to_string (
    const std::array<uint8_t, 6>& mac) {
            
    std::ostringstream oss;

    for (auto byte : mac) {
        oss << utils::byte_to_hex(byte);
    }

    return oss.str();
}

std::vector<uint8_t> utils::ascii_to_bytes (std::string_view ascii_str) {
    return std::vector<uint8_t>(ascii_str.begin(), ascii_str.end());
}

std::string utils::byte_to_bit_string (uint8_t byte) {
    std::string bits;
    for (int i = 7; i >= 0; --i) {
        bits += ((byte >> i) & 1) ? '1' : '0';
    }
    return bits;
}

std::string utils::byte_to_bit_string(uint16_t value) {
    std::string bits;
    for (int i = 15; i >= 0; --i) {
        bits += ((value >> i) & 1) ? '1' : '0';
    }
    return bits;
}

std::string utils::bytes_to_bit_string(const std::vector<uint8_t>& bytes) {
    std::string bits;
    bits.reserve(bytes.size() * 8);
    for (uint8_t b : bytes) {
        bits += byte_to_bit_string(b);
    }
    return bits;
}
