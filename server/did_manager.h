#ifndef DID_MANAGER_H
#define DID_MANAGER_H

#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include "uds_protocol.h"

namespace uds {

class DIDManager {
public:
    DIDManager(const std::string& data_file_path);
    ~DIDManager() = default;
    
    // 加载DID数据
    bool load_data();
    
    // 保存DID数据
    bool save_data();
    
    // 读取DID值
    bool read_did(DID did, std::vector<uint8_t>& data);
    
    // 写入DID值
    bool write_did(DID did, const std::vector<uint8_t>& data);
    
private:
    // 从JSON字符串中解析DID数据
    bool parse_json(const std::string& json_str);
    
    // 生成JSON字符串
    std::string generate_json() const;
    
    std::string data_file_path_;
    // 存储DID数据：key为DID，value为数据字节数组
    std::map<DID, std::vector<uint8_t>> did_data_;
};

} // namespace uds

#endif // DID_MANAGER_H
