
#pragma once

#include "frame_type.h"

namespace utils {

ethernet::ErrorCode 
    parse_mac (const std::string& mac_str, 
            std::vector<uint8_t>& mac,
            bool is_dest = true);

std::vector<uint8_t> ascii_to_bytes (const std::string& ascii_str);

std::string bytes_to_bitstring (const std::vector<uint8_t>& data);
}