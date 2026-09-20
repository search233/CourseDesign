#pragma once

#include "frame_type.h"


namespace ethernet::crc {

// G(X) = X^8 + X^2 + X + 1
// 0001 0000 0111
inline constexpr uint16_t POLY = 0x0107;


uint8_t calculate_crc8(
        const std::vector<uint8_t>& data, 
        std::vector<CrcSnapshot>& trace,
        uint16_t poly = POLY
    );
    
}

