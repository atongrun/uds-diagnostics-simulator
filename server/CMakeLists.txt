cmake_minimum_required(VERSION 3.10)

project(UDSServer VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(uds_server 
    uds_server.cpp
    uds_protocol.cpp
    did_manager.cpp
)

# 包含头文件目录
target_include_directories(uds_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 链接库（Windows下需要链接ws2_32.lib）
if(WIN32)
    target_link_libraries(uds_server ws2_32)
endif()
