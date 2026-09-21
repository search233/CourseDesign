#include "frame_builder.h"
#include "frame_type.h"
#include "crc_engine.h"
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
    if (utils::parse_mac(dest_mac, res.frame.destmac) == false) {
        set_status(ErrorCode::InvalidDestMac);
        return res;
    }

    // 解析源 mac
    if (utils::parse_mac(src_mac, res.frame.destmac) == false) {
        set_status(ErrorCode::InvalidSrcMac);
        return res;
    }

    // 处理 payload 数据
    std::vector<uint8_t> payload_bytes = 
        utils::ascii_to_bytes(payload);    
    size_t payload_length = payload_bytes.size();
    

    // 判断是否超限
    if (payload_length > 1500) {
        set_status(ErrorCode::PayloadTooLarge);
        return res;
    }

    res.frame.payload = std::move(payload_bytes);
    res.frame.payload_length = static_cast<uint16_t>(payload_length);


    // 判断并填充
    if (payload_length < 46) {
        res.frame.padding.assign(46 - payload_length, 0x00);
    }

    // 计算 crc
    uint8_t crc_val = crc::calculate_crc8(
        res.frame.get_crc_payload(),
        res.crc_snaps
    );
    res.frame.fcs = static_cast<uint32_t>(crc_val);

    set_status(ErrorCode::Success);
    return res;

}
}


