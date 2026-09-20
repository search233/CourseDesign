#include "frame_type.h"

std::string 
    ethernet::error_to_string (ethernet::ErrorCode err) {

    std::string res;    

    switch (err) {
        using enum ErrorCode;

        case Success: {
            res = "封装成功";
            break;
        }

        case InvalidDestMac: {
            res = "目的 MAC 地址格式错误";
            break;
        }

        case InvalidSrcMac: {
            res = "源 MAC 地址格式错误";
            break;
        }

        case PayloadTooLarge: {
            res = "有效载荷超出最大限制";
            break;
        }

        default: {
            res = "未知错误";
        }
    }

    return res;
};

using eeframe = ethernet::EthernetFrame;

size_t eeframe::get_size() const {

    return 
        preamble.size() 
        + 1  // sfd
        + destmac.size()
        + srcmac.size()
        + 2  // 长度字段
        + payload.size()
        + padding.size()
        + 4;  // 校验字段
}

std::vector<uint8_t> eeframe::serialize() const {

    std::vector<uint8_t> bytes;
    bytes.reserve(eeframe::get_size());

    // 填充前导码, sfd
    bytes.insert(bytes.end(), preamble.begin(), preamble.end());
    bytes.push_back(sfd);

    // 填充两段 mac
    bytes.insert(bytes.end(), destmac.begin(), destmac.end());
    bytes.insert(bytes.end(), srcmac.begin(), srcmac.end());

    // 长度字段
    bytes.push_back(static_cast<uint8_t>(payload_length >> 8) & 0xff);
    bytes.push_back(static_cast<uint8_t>(payload_length & 0xff));

    // 数据 补充字段
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    bytes.insert(bytes.end(), padding.begin(), padding.end());

    // 校验字段
    bytes.push_back(static_cast<uint8_t>(fcs >> 24) & 0xff);
    bytes.push_back(static_cast<uint8_t>(fcs >> 16) & 0xff);
    bytes.push_back(static_cast<uint8_t>(fcs >> 8) & 0xff);
    bytes.push_back(static_cast<uint8_t>(fcs & 0xff));

    return bytes;
}

std::vector<uint8_t> eeframe::get_crc_payload() const {
    std::vector<uint8_t> bytes; 
    size_t bytes_size = 6 + 6 + 2 + payload.size() + padding.size();
    bytes.reserve(bytes_size);

    // 填充两段 mac
    bytes.insert(bytes.end(), destmac.begin(), destmac.end());
    bytes.insert(bytes.end(), srcmac.begin(), srcmac.end());

    // 长度字段
    bytes.push_back(static_cast<uint8_t>(payload_length >> 8) & 0xff);
    bytes.push_back(static_cast<uint8_t>(payload_length & 0xff));

    // 数据 补充字段
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    bytes.insert(bytes.end(), padding.begin(), padding.end());

    return bytes;
}

std::vector<ethernet::FieldSpan> eeframe::get_field_spans() const {
    std::vector<ethernet::FieldSpan> spans;
    size_t offset = 0;

    auto add_span = [&](FrameSegmentType tp, size_t length) -> void {
        if (length > 0) {
            spans.push_back({tp, offset, length});
            offset += length;
        }
    };

    add_span(FrameSegmentType::Preamble, preamble.size());
    add_span(FrameSegmentType::SFD, sizeof(sfd));
    add_span(FrameSegmentType::DestMac, destmac.size());
    add_span(FrameSegmentType::SrcMac, srcmac.size());
    add_span(FrameSegmentType::Length, sizeof(payload_length));
    add_span(FrameSegmentType::Payload, payload.size());
    add_span(FrameSegmentType::Padding, padding.size());
    add_span(FrameSegmentType::FCS, sizeof(fcs));

    return spans;
}

