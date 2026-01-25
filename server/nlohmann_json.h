// 简化版的JSON处理，用于DID数据存储
// 实际项目中建议使用完整的nlohmann/json库

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <cctype>

namespace json {

class Value {
public:
    enum Type {
        NULL_T,
        BOOL_T,
        NUMBER_T,
        STRING_T,
        ARRAY_T,
        OBJECT_T
    };

    Value() : type_(NULL_T) {}
    Value(bool b) : type_(BOOL_T), bool_value_(b) {}
    Value(int i) : type_(NUMBER_T), number_value_(i) {}
    Value(double d) : type_(NUMBER_T), number_value_(d) {}
    Value(const std::string& s) : type_(STRING_T), string_value_(s) {}
    
    // 对象和数组访问
    std::map<std::string, Value>& get_object() { return object_value_; }
    std::vector<Value>& get_array() { return array_value_; }
    
    // 设置值
    void set_string(const std::string& s) {
        type_ = STRING_T;
        string_value_ = s;
    }
    
    void set_number(double d) {
        type_ = NUMBER_T;
        number_value_ = d;
    }
    
    // 类型检查
    Type get_type() const { return type_; }
    bool is_object() const { return type_ == OBJECT_T; }
    bool is_array() const { return type_ == ARRAY_T; }
    bool is_string() const { return type_ == STRING_T; }
    bool is_number() const { return type_ == NUMBER_T; }
    
private:
    Type type_;
    bool bool_value_;
    double number_value_;
    std::string string_value_;
    std::vector<Value> array_value_;
    std::map<std::string, Value> object_value_;
};

class Parser {
public:
    Parser(const std::string& json_str) : json_str_(json_str), pos_(0) {}
    
    Value parse() {
        skip_whitespace();
        return parse_value();
    }
    
private:
    void skip_whitespace() {
        while (pos_ < json_str_.size() && std::isspace(json_str_[pos_])) {
            pos_++;
        }
    }
    
    Value parse_value() {
        skip_whitespace();
        if (json_str_[pos_] == '{') {
            return parse_object();
        } else if (json_str_[pos_] == '[') {
            return parse_array();
        } else if (json_str_[pos_] == '"') {
            return parse_string();
        } else if (json_str_[pos_] == 't' || json_str_[pos_] == 'f') {
            return parse_bool();
        } else if (json_str_[pos_] == 'n') {
            return parse_null();
        } else {
            return parse_number();
        }
    }
    
    Value parse_object() {
        Value obj;
        pos_++; // skip '{'
        skip_whitespace();
        
        while (pos_ < json_str_.size() && json_str_[pos_] != '}') {
            std::string key = parse_string().string_value_;
            skip_whitespace();
            pos_++; // skip ':'
            Value value = parse_value();
            obj.object_value_[key] = value;
            skip_whitespace();
            if (json_str_[pos_] == ',') {
                pos_++;
                skip_whitespace();
            }
        }
        
        pos_++; // skip '}'
        return obj;
    }
    
    Value parse_array() {
        Value arr;
        pos_++; // skip '['
        skip_whitespace();
        
        while (pos_ < json_str_.size() && json_str_[pos_] != ']') {
            Value value = parse_value();
            arr.array_value_.push_back(value);
            skip_whitespace();
            if (json_str_[pos_] == ',') {
                pos_++;
                skip_whitespace();
            }
        }
        
        pos_++; // skip ']'
        return arr;
    }
    
    Value parse_string() {
        std::string str;
        pos_++; // skip '"'
        while (pos_ < json_str_.size() && json_str_[pos_] != '"') {
            if (json_str_[pos_] == '\\') {
                pos_++;
                if (pos_ < json_str_.size()) {
                    str += json_str_[pos_];
                    pos_++;
                }
            } else {
                str += json_str_[pos_];
                pos_++;
            }
        }
        pos_++; // skip '"'
        return Value(str);
    }
    
    Value parse_number() {
        double num = 0.0;
        bool is_negative = false;
        if (json_str_[pos_] == '-') {
            is_negative = true;
            pos_++;
        }
        
        while (pos_ < json_str_.size() && std::isdigit(json_str_[pos_])) {
            num = num * 10.0 + (json_str_[pos_] - '0');
            pos_++;
        }
        
        if (is_negative) {
            num = -num;
        }
        
        return Value(num);
    }
    
    Value parse_bool() {
        if (json_str_.substr(pos_, 4) == "true") {
            pos_ += 4;
            return Value(true);
        } else if (json_str_.substr(pos_, 5) == "false") {
            pos_ += 5;
            return Value(false);
        }
        return Value(false);
    }
    
    Value parse_null() {
        pos_ += 4; // skip 'null'
        return Value();
    }
    
    std::string json_str_;
    size_t pos_;
};

class Serializer {
public:
    static std::string serialize(const Value& value, int indent = 0) {
        switch (value.type_) {
            case Value::OBJECT_T:
                return serialize_object(value, indent);
            case Value::ARRAY_T:
                return serialize_array(value, indent);
            case Value::STRING_T:
                return '"' + value.string_value_ + '"';
            case Value::NUMBER_T:
                return std::to_string(value.number_value_);
            case Value::BOOL_T:
                return value.bool_value_ ? "true" : "false";
            default:
                return "null";
        }
    }
    
private:
    static std::string serialize_object(const Value& value, int indent) {
        if (value.object_value_.empty()) {
            return "{}";
        }
        
        std::string result = "{\n";
        std::string indent_str(indent + 2, ' ');
        
        auto it = value.object_value_.begin();
        while (true) {
            result += indent_str + '"' + it->first + '": ' + serialize(it->second, indent + 2);
            ++it;
            if (it != value.object_value_.end()) {
                result += ",\n";
            } else {
                break;
            }
        }
        
        result += '\n' + std::string(indent, ' ') + '}';
        return result;
    }
    
    static std::string serialize_array(const Value& value, int indent) {
        if (value.array_value_.empty()) {
            return "[]";
        }
        
        std::string result = "[\n";
        std::string indent_str(indent + 2, ' ');
        
        auto it = value.array_value_.begin();
        while (true) {
            result += indent_str + serialize(*it, indent + 2);
            ++it;
            if (it != value.array_value_.end()) {
                result += ",\n";
            } else {
                break;
            }
        }
        
        result += '\n' + std::string(indent, ' ') + ']';
        return result;
    }
};

} // namespace json
