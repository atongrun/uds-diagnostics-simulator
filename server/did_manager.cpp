#include "did_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace uds {

DIDManager::DIDManager(const std::string& data_file_path)
    : data_file_path_(data_file_path) {
    // 构造函数中尝试加载数据
    load_data();
}

// 从JSON字符串中解析DID数据
bool DIDManager::parse_json(const std::string& json_str) {
    did_data_.clear();
    
    // 简单的JSON解析，仅处理我们需要的格式
    size_t dids_start = json_str.find("\"dids\": {");
    if (dids_start == std::string::npos) {
        return false;
    }
    
    dids_start += 8; // 跳过 "dids": {
    size_t dids_end = json_str.rfind("}");
    if (dids_end == std::string::npos || dids_end <= dids_start) {
        return false;
    }
    
    // 解析每个DID条目
    std::string dids_str = json_str.substr(dids_start, dids_end - dids_start);
    size_t pos = 0;
    
    while (pos < dids_str.size()) {
        // 跳过空白字符
        while (pos < dids_str.size() && std::isspace(dids_str[pos])) {
            pos++;
        }
        
        // 查找DID键（"XXXX":）
        size_t did_key_start = dids_str.find('"', pos);
        if (did_key_start == std::string::npos) {
            break;
        }
        
        size_t did_key_end = dids_str.find('"', did_key_start + 1);
        if (did_key_end == std::string::npos) {
            break;
        }
        
        // 解析DID
        std::string did_str = dids_str.substr(did_key_start + 1, did_key_end - did_key_start - 1);
        DID did = static_cast<DID>(std::stoi(did_str, nullptr, 16));
        
        // 查找数据数组（[ ... ]）
        size_t array_start = dids_str.find('[', did_key_end);
        if (array_start == std::string::npos) {
            break;
        }
        
        size_t array_end = dids_str.find(']', array_start);
        if (array_end == std::string::npos) {
            break;
        }
        
        // 解析数据数组
        std::vector<uint8_t> data;
        std::string array_str = dids_str.substr(array_start + 1, array_end - array_start - 1);
        std::istringstream array_iss(array_str);
        std::string byte_str;
        
        while (array_iss >> byte_str) {
            // 移除逗号
            if (byte_str.back() == ',') {
                byte_str.pop_back();
            }
            
            try {
                uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 10));
                data.push_back(byte);
            } catch (...) {
                // 忽略无效数据
            }
        }
        
        // 添加到DID数据映射
        did_data_[did] = data;
        
        // 移动到下一个条目
        pos = array_end + 1;
    }
    
    return true;
}

// 生成JSON字符串
std::string DIDManager::generate_json() const {
    std::stringstream json_ss;
    
    json_ss << "{" << std::endl;
    json_ss << "  \"version\": \"1.0\"," << std::endl;
    json_ss << "  \"description\": \"UDS DID Data\"," << std::endl;
    json_ss << "  \"dids\": {" << std::endl;
    
    // 写入所有DID数据
    auto it = did_data_.begin();
    while (it != did_data_.end()) {
        // 格式化DID为4位十六进制字符串
        std::stringstream did_ss;
        did_ss << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << it->first;
        std::string did_str = did_ss.str();
        
        // 写入DID键
        json_ss << "    \"" << did_str << "\": [";
        
        // 写入数据数组
        const auto& data = it->second;
        for (size_t i = 0; i < data.size(); ++i) {
            if (i > 0) {
                json_ss << ", ";
            }
            json_ss << static_cast<int>(data[i]);
        }
        
        json_ss << "]";
        
        // 检查是否是最后一个条目
        ++it;
        if (it != did_data_.end()) {
            json_ss << ",";
        }
        json_ss << std::endl;
    }
    
    json_ss << "  }" << std::endl;
    json_ss << "}" << std::endl;
    
    return json_ss.str();
}

// 加载DID数据
bool DIDManager::load_data() {
    std::ifstream file(data_file_path_);
    if (!file.is_open()) {
        std::cerr << "Failed to open DID data file: " << data_file_path_ << std::endl;
        // 如果文件不存在，创建默认数据
        did_data_[0x1234] = {0x01, 0x02, 0x03, 0x04}; // 示例DID 1234
        did_data_[0x5678] = {0xAA, 0xBB, 0xCC, 0xDD}; // 示例DID 5678
        did_data_[0x0001] = {0x56, 0x31, 0x2E, 0x30, 0x2E, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 版本号 V1.0.0
        did_data_[0x0002] = {0x00, 0x64}; // 车速 100 km/h
        did_data_[0x0003] = {0x03, 0xE8}; // 发动机转速 1000 rpm
        did_data_[0x0004] = {0x00, 0x00, 0x00, 0x01}; // 功能配置字
        save_data();
        return true;
    }
    
    // 读取整个文件内容
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_str = buffer.str();
    
    // 解析JSON
    if (!parse_json(json_str)) {
        std::cerr << "Failed to parse DID data file: " << data_file_path_ << std::endl;
        file.close();
        return false;
    }
    
    file.close();
    return true;
}

// 保存DID数据
bool DIDManager::save_data() {
    std::ofstream file(data_file_path_);
    if (!file.is_open()) {
        std::cerr << "Failed to open DID data file for writing: " << data_file_path_ << std::endl;
        return false;
    }
    
    // 生成JSON字符串
    std::string json_str = generate_json();
    
    // 写入文件
    file << json_str;
    
    file.close();
    return true;
}

// 读取DID值
bool DIDManager::read_did(DID did, std::vector<uint8_t>& data) {
    auto it = did_data_.find(did);
    if (it != did_data_.end()) {
        data = it->second;
        return true;
    }
    return false;
}

// 写入DID值
bool DIDManager::write_did(DID did, const std::vector<uint8_t>& data) {
    did_data_[did] = data;
    return save_data();
}

} // namespace uds
