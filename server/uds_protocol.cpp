#include "uds_protocol.h"

namespace uds {

// 解析UDS请求报文
UdsMessage parse_request(const std::vector<uint8_t>& raw_message) {
    UdsMessage message;
    message.is_positive_response = false;
    message.response_code = ResponseCode::POSITIVE_RESPONSE;
    
    if (raw_message.empty()) {
        message.is_positive_response = false;
        message.response_code = ResponseCode::INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT;
        return message;
    }
    
    // 解析服务ID
    message.service_id = static_cast<ServiceID>(raw_message[0]);
    
    // 解析DID（如果有）
    if (raw_message.size() >= 3) {
        message.did = bytes_to_did(raw_message, 1);
    }
    
    // 解析数据（如果有）
    if (raw_message.size() > 3) {
        message.data.assign(raw_message.begin() + 3, raw_message.end());
    }
    
    return message;
}

// 生成UDS响应报文
std::vector<uint8_t> generate_response(const UdsMessage& message) {
    std::vector<uint8_t> response;
    
    if (message.is_positive_response) {
        // 正响应：服务ID + 0x40
        uint8_t response_service_id = static_cast<uint8_t>(message.service_id) + 0x40;
        response.push_back(response_service_id);
        
        // 添加DID
        auto did_bytes = did_to_bytes(message.did);
        response.insert(response.end(), did_bytes.begin(), did_bytes.end());
        
        // 添加响应数据
        response.insert(response.end(), message.data.begin(), message.data.end());
    } else {
        // 负响应：0x7F + 原始服务ID + 响应码
        response.push_back(static_cast<uint8_t>(ResponseCode::NEGATIVE_RESPONSE));
        response.push_back(static_cast<uint8_t>(message.service_id));
        response.push_back(static_cast<uint8_t>(message.response_code));
    }
    
    return response;
}

// 辅助函数：将字节数组转换为DID
DID bytes_to_did(const std::vector<uint8_t>& bytes, size_t offset) {
    if (bytes.size() < offset + 2) {
        return 0;
    }
    return static_cast<DID>((bytes[offset] << 8) | bytes[offset + 1]);
}

// 辅助函数：将DID转换为字节数组
std::vector<uint8_t> did_to_bytes(DID did) {
    std::vector<uint8_t> bytes;
    bytes.push_back(static_cast<uint8_t>((did >> 8) & 0xFF));
    bytes.push_back(static_cast<uint8_t>(did & 0xFF));
    return bytes;
}

} // namespace uds
