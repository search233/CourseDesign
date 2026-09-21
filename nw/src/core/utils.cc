#include "utils.h"
#include <cassert>
#include <sstream>

ethernet::ErrorCode
    utils::parse_mac (
        const std::string& mac_str,
        std::vector<uint8_t>& mac,
        bool is_dest) {

    // 0: success
    // 1: invalid dest
    // 2: invalid src
    unsigned int tag = 0;
    auto put_tag = [&]() -> void {
        if (is_dest) tag = 1;
        else tag = 2;
    };

    int len = mac_str.length();
    std::string clean_str;
    for (int i = 0; i < len; ++i) {
        char ch = mac_str[i];

        if (ch == '-' || ch == ':' || ch == ' ') {
            if ((i + 1) % 3 == 0) continue;
            else {
                put_tag();
                break;
            }
        }

        if (!std::isxdigit(static_cast<unsigned char>(ch))) {
            put_tag();
            break;
        }

        clean_str += ch;
    }

    if (clean_str.length() != 12) {
        put_tag();
    }

    switch (tag) {
        using err_code = ethernet::ErrorCode;
        case 0: {
            return err_code::Success;
        }

        case 1: {
            return err_code::InvalidDestMac;
        }

        case 2: {
            return err_code::InvalidSrcMac;
        }

        default: {
            assert(tag <= 2);
        }
    }

    return ethernet::ErrorCode::Success;
}

std::string utils::byte_to_hex (uint8_t byte) {
    std::ostringstream oss;

    oss << std::hex 
        << std::uppercase 
        << std::setfill('0') 
        << std::setw(2) 
        << static_cast<int>(byte);

    return oss.str();
}
