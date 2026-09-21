#pragma once

#include "frame_type.h"
#include <string_view>

namespace ethernet::builder {

Result encapsulate(
    std::string_view dest_mac,
    std::string_view src_mac,
    std::string_view payload
);

} // namespace ethernet::builder
