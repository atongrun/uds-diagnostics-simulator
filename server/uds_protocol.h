#ifndef UDS_PROTOCOL_H
#define UDS_PROTOCOL_H

#include <vector>
#include <cstdint>

namespace uds {

// UDS服务ID
enum class ServiceID : uint8_t {
    READ_DATA_BY_IDENTIFIER = 0x22,
    WRITE_DATA_BY_IDENTIFIER = 0x2E
};

// UDS响应码
enum class ResponseCode : uint8_t {
    POSITIVE_RESPONSE = 0x7F,  // 注意：正响应是服务ID+0x40
    NEGATIVE_RESPONSE = 0x7F,
    // 负响应码
    SERVICE_NOT_SUPPORTED = 0x11,
    SUB_FUNCTION_NOT_SUPPORTED = 0x12,
    INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT = 0x13,
    REQUEST_OUT_OF_RANGE = 0x31,
    SECURITY_ACCESS_DENIED = 0x33,
    INVALID_KEY = 0x35,
    EXCEEDED_NUMBER_OF_ATTEMPTS = 0x36,
    REQUIRED_TIME_DELAY_NOT_EXPIRED = 0x37,
    UPLOAD_DOWNLOAD_NOT_ACCEPTED = 0x70,
    TRANSFER_DATA_SUSPENDED = 0x71,
    GENERAL_PROGRAMMING_FAILURE = 0x72,
    WRONG_BLOCK_SEQUENCE_COUNTER = 0x73,
    REQUEST_CORRECTLY_RECEIVED_BUT_RESPONSE_PENDING = 0x78,
    SUB_FUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7E,
    SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7F
};

// DID类型定义
typedef uint16_t DID;

// UDS报文结构
struct UdsMessage {
    ServiceID service_id;
    DID did;
    std::vector<uint8_t> data;
    bool is_positive_response;
    ResponseCode response_code;
};

// 解析UDS请求报文
UdsMessage parse_request(const std::vector<uint8_t>& raw_message);

// 生成UDS响应报文
std::vector<uint8_t> generate_response(const UdsMessage& message);

// 辅助函数：将字节数组转换为DID
DID bytes_to_did(const std::vector<uint8_t>& bytes, size_t offset = 0);

// 辅助函数：将DID转换为字节数组
std::vector<uint8_t> did_to_bytes(DID did);

} // namespace uds

#endif // UDS_PROTOCOL_H
