#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <map>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET SocketType;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define CLOSE_SOCKET(s) closesocket(s)
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SocketType;
    #define INVALID_SOCKET_VALUE -1
    #define CLOSE_SOCKET(s) close(s)
    #define SOCKET_ERROR_VALUE -1
#endif

#include "uds_protocol.h"
#include "did_manager.h"

using namespace uds;

class UDSServer {
public:
    UDSServer(int port, const std::string& data_file_path)
        : port_(port), did_manager_(data_file_path) {
    }
    
    ~UDSServer() {
        stop();
    }
    
    bool start() {
        #ifdef _WIN32
        // 初始化Winsock
        WSADATA wsa_data;
        int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return false;
        }
        #endif
        
        // 创建socket
        server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket_ == INVALID_SOCKET_VALUE) {
            std::cerr << "Failed to create socket" << std::endl;
            #ifdef _WIN32
            WSACleanup();
            #endif
            return false;
        }
        
        // 设置socket选项：允许地址重用
        int opt = 1;
        if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, 
                      reinterpret_cast<const char*>(&opt), sizeof(opt)) < 0) {
            std::cerr << "setsockopt failed" << std::endl;
            CLOSE_SOCKET(server_socket_);
            #ifdef _WIN32
            WSACleanup();
            #endif
            return false;
        }
        
        // 绑定socket到地址和端口
        sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port_);
        
        if (bind(server_socket_, reinterpret_cast<struct sockaddr*>(&server_addr), 
                sizeof(server_addr)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            CLOSE_SOCKET(server_socket_);
            #ifdef _WIN32
            WSACleanup();
            #endif
            return false;
        }
        
        // 开始监听
        if (listen(server_socket_, 5) < 0) {
            std::cerr << "Listen failed" << std::endl;
            CLOSE_SOCKET(server_socket_);
            #ifdef _WIN32
            WSACleanup();
            #endif
            return false;
        }
        
        std::cout << "UDS Server started, listening on port " << port_ << std::endl;
        
        // 接受连接（单线程版本）
        is_running_ = true;
        accept_connections();
        
        return true;
    }
    
    void stop() {
        is_running_ = false;
        
        // 关闭服务器socket
        if (server_socket_ != INVALID_SOCKET_VALUE) {
            CLOSE_SOCKET(server_socket_);
            server_socket_ = INVALID_SOCKET_VALUE;
        }
        
        #ifdef _WIN32
        WSACleanup();
        #endif
        
        std::cout << "UDS Server stopped" << std::endl;
    }
    
private:
    void accept_connections() {
        while (is_running_) {
            sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            
            // 接受客户端连接
            SocketType client_socket = accept(server_socket_, 
                                            reinterpret_cast<struct sockaddr*>(&client_addr), 
                                            &client_addr_len);
            
            if (client_socket == INVALID_SOCKET_VALUE) {
                if (is_running_) {
                    std::cerr << "Accept failed" << std::endl;
                }
                continue;
            }
            
            // 获取客户端IP地址（使用inet_ntoa替代inet_ntop）
            char* client_ip = inet_ntoa(client_addr.sin_addr);
            std::cout << "New client connected: " << client_ip << std::endl;
            
            // 处理客户端请求（单线程版本）
            handle_client(client_socket, client_ip);
        }
    }
    
    void handle_client(SocketType client_socket, const char* client_ip) {
        const int BUFFER_SIZE = 1024;
        char buffer[BUFFER_SIZE];
        
        while (is_running_) {
            // 接收客户端数据
            std::memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    std::cout << "Client disconnected: " << client_ip << std::endl;
                } else {
                    std::cerr << "Receive failed from client: " << client_ip << std::endl;
                }
                break;
            }
            
            // 处理接收到的数据
            std::vector<uint8_t> request_data(buffer, buffer + bytes_received);
            std::vector<uint8_t> response_data = process_request(request_data);
            
            // 发送响应
            int bytes_sent = send(client_socket, reinterpret_cast<const char*>(response_data.data()), 
                                response_data.size(), 0);
            
            if (bytes_sent < 0) {
                std::cerr << "Send failed to client: " << client_ip << std::endl;
                break;
            }
        }
        
        // 关闭客户端socket
        CLOSE_SOCKET(client_socket);
    }
    
    std::vector<uint8_t> process_request(const std::vector<uint8_t>& request_data) {
        // 解析UDS请求
        UdsMessage request = parse_request(request_data);
        UdsMessage response = request;
        response.is_positive_response = true;
        
        // 处理不同的服务
        switch (request.service_id) {
            case ServiceID::READ_DATA_BY_IDENTIFIER:
                handle_read_data_by_identifier(request, response);
                break;
            
            case ServiceID::WRITE_DATA_BY_IDENTIFIER:
                handle_write_data_by_identifier(request, response);
                break;
            
            default:
                response.is_positive_response = false;
                response.response_code = ResponseCode::SERVICE_NOT_SUPPORTED;
                break;
        }
        
        // 生成响应报文
        return generate_response(response);
    }
    
    void handle_read_data_by_identifier(const UdsMessage& request, UdsMessage& response) {
        std::vector<uint8_t> data;
        if (did_manager_.read_did(request.did, data)) {
            response.data = data;
            response.is_positive_response = true;
        } else {
            response.is_positive_response = false;
            response.response_code = ResponseCode::REQUEST_OUT_OF_RANGE;
        }
    }
    
    void handle_write_data_by_identifier(const UdsMessage& request, UdsMessage& response) {
        if (did_manager_.write_did(request.did, request.data)) {
            response.data = request.data;
            response.is_positive_response = true;
        } else {
            response.is_positive_response = false;
            response.response_code = ResponseCode::GENERAL_PROGRAMMING_FAILURE;
        }
    }
    
    int port_;
    SocketType server_socket_ = INVALID_SOCKET_VALUE;
    bool is_running_ = false;
    DIDManager did_manager_;
};

int main(int argc, char* argv[]) {
    int port = 8888;
    std::string data_file_path = "../data/did_data.json";
    
    // 解析命令行参数
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    
    if (argc > 2) {
        data_file_path = argv[2];
    }
    
    UDSServer server(port, data_file_path);
    
    if (!server.start()) {
        std::cerr << "Failed to start UDS Server" << std::endl;
        return 1;
    }
    
    std::cout << "Press Enter to stop the server..." << std::endl;
    std::cin.get();
    
    server.stop();
    
    return 0;
}
