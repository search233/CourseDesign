#include "frame_builder.h"
#include "frame_type.h"
#include "utils.h"
#include <cstddef>
#include <cstdint>
#include <vector>



namespace ethernet::builder {

Result encapsulate(
    std::string_view dest_mac,
    std::string_view src_mac,
    std::string_view payload) {

    Result res;


    auto set_status = [&](ErrorCode code) -> void {
        res.status = code;
        res.error_message = error_to_string(code);
    };

    // 解析目标 mac
    if (!utils::parse_mac(dest_mac, res.frame.destmac) == false) {
        set_status(ErrorCode::InvalidDestMac);
        return res;
    }

    // 解析源 mac
    res.status = utils::parse_mac(src_mac, res.frame.destmac, false);
    if (res.status != ErrorCode::Success) {
        res.error_message = error_to_string(res.status);
        return res;
    }

    // 处理数据
    std::vector<uint8_t> payload_bytes = 
        utils::ascii_to_bytes(payload);    
    size_t payload_length = payload_bytes.size();
    res.frame.payload_length = payload_length;

    // 判断是否超限
    if (payload_length > 1500) {
        res.status = 
    }

    // 填充
    if (payload_length < 46) {
        res.frame.padding.assign(46 - payload_length, 0x00);
    }

    

    

    return res;
}
}


