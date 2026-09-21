#include "utils.h"
#include <cassert>
#include <sstream>
#include <iomanip>

bool parse_mac (
    const std::string& mac_str,
    std::array<uint8_t, 6>& mac) {

    bool tag = 1;

    int len = mac_str.length();
    std::string clean_str;
    for (int i = 0; i < len; ++i) {
        char ch = mac_str[i];

        if (ch == '-' || ch == ':' || ch == ' ') {
            if ((i + 1) % 3 == 0) continue;
            else {
                tag &= 0;
                break;
            }
        }

        if (!std::isxdigit(static_cast<unsigned char>(ch))) {
            tag &= 0;
            break;
        }

        clean_str += ch;
    }

    if (clean_str.length() != 12) {
        tag &= 0;
    }

    return static_cast<bool>(tag);
}

std::string byte_to_hex (uint8_t byte) {
    std::ostringstream oss;

    oss << std::hex 
        << std::uppercase 
        << std::setfill('0') 
        << std::setw(2)
        << static_cast<int>(byte);

    return oss.str();
}


std::string mac_to_string (
    const std::array<uint8_t, 6>& mac) {
            
    std::ostringstream oss;

    for (auto byte : mac) {
        oss << utils::byte_to_hex(byte);
    }

    return oss.str();
}

std::vector<uint8_t> ascii_to_bytes (const std::string& ascii_str) {
    return std::vector<uint8_t>(ascii_str.begin(), ascii_str.end());
}

std::string byte_to_bit_string (uint8_t byte) {
    std::string bits;
    for (int i = 7; i >= 0; --i) {
        bits += ((byte >> i) & 1) ? '1' : '0';
    }
    return bits;
}

std::string uint16_to_bit_string(uint16_t value) {
    std::string bits;
    for (int i = 15; i >= 0; --i) {
        bits += ((value >> i) & 1) ? '1' : '0';
    }
    return bits;
}

std::string bytes_to_bit_string(const std::vector<uint8_t>& bytes) {
    std::string bits;
    bits.reserve(bytes.size() * 8);
    for (uint8_t b : bytes) {
        bits += byte_to_bit_string(b);
    }
    return bits;
}
