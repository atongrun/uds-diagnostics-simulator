# UDS诊断模拟器 (UDS Diagnostics Simulator)

## 项目概述

UDS诊断模拟器是一个基于C++和Web技术的统一诊断服务（UDS）模拟系统，支持22（读取DID）和2E（写入DID）服务，通过TCP/IP协议模拟真实的UDS报文通信。

## 核心功能

- **UDS协议支持**：实现22服务（读取DID）和2E服务（写入DID）
- **TCP/IP通信**：模拟真实UDS报文传输
- **JSON数据存储**：通过JSON文件保存和读取DID数据
- **网页客户端**：提供简洁的UI界面发送诊断指令
- **WebSocket-TCP桥接**：支持浏览器与TCP服务端通信
- **跨平台支持**：兼容Windows和Linux平台

## 项目结构

```
uds-diagnostics-simulator/
├── client/              # 网页客户端代码
│   ├── index.html       # 主页面
│   ├── style.css        # 样式文件
│   └── script.js        # JavaScript逻辑
├── data/                # 数据存储目录
│   └── did_data.json    # DID数据文件（JSON格式）
├── server/              # C++服务端代码
│   ├── uds_server_simple.cpp  # 简化版主服务端程序
│   ├── uds_protocol.h   # UDS协议定义
│   ├── uds_protocol.cpp # UDS协议实现
│   ├── did_manager.h    # DID管理接口
│   ├── did_manager.cpp  # DID管理实现（JSON存储）
│   └── CMakeLists.txt   # CMake构建脚本
├── websocket_bridge.js  # WebSocket-TCP桥接服务
├── package.json         # Node.js依赖配置
├── README.md            # 项目说明文档
└── docs/                # 详细文档目录
```

## 安装与运行

### 1. 服务端编译

#### 使用CMake（推荐）
```bash
cd server
mkdir build
cd build
cmake ..
cmake --build .
```

#### 直接编译（Windows）
```bash
cd server
g++ -std=c++11 uds_server_simple.cpp uds_protocol.cpp did_manager.cpp -o uds_server -lws2_32
```

#### 直接编译（Linux）
```bash
cd server
g++ -std=c++11 uds_server_simple.cpp uds_protocol.cpp did_manager.cpp -o uds_server
```

### 2. 启动服务端

```bash
cd server
./uds_server  # Linux/macOS
.\uds_server.exe  # Windows
```

服务端默认监听8888端口。

### 3. 启动WebSocket-TCP桥接服务

```bash
# 安装依赖（首次运行）
npm install

# 启动桥接服务
node websocket_bridge.js
```

桥接服务默认监听8080端口，转发到TCP端口8888。

### 4. 打开网页客户端

- **直接打开**：在浏览器中直接打开 `client/index.html`
- **使用本地HTTP服务器**：
  ```bash
  # Python 3
  python -m http.server 8000
  # 然后在浏览器中访问 http://localhost:8000/client/
  ```

## 使用示例

### 读取DID（22服务）

1. 在网页客户端中：
   - 连接到服务器（IP: 127.0.0.1, 端口: 8080）
   - 选择服务：`22 - 读取DID`
   - 输入DID：`1234`
   - 点击"发送指令"

2. 预期结果：
   - 请求报文：`22 12 34`
   - 响应报文：`62 12 34 01 02 03 04`
   - 响应状态：`正响应 - 服务: 0x22`

### 写入DID（2E服务）

1. 在网页客户端中：
   - 连接到服务器（IP: 127.0.0.1, 端口: 8080）
   - 选择服务：`2E - 写入DID`
   - 输入DID：`5678`
   - 输入数据：`AA BB CC DD`
   - 点击"发送指令"

2. 预期结果：
   - 请求报文：`2E 56 78 AA BB CC DD`
   - 响应报文：`6E 56 78 AA BB CC DD`
   - 响应状态：`正响应 - 服务: 0x2E`

## 支持的DID列表

| DID | 描述 | 类型 | 初始值 |
|-----|------|------|--------|
| 1234 | 示例DID 1 | 读写 | 01 02 03 04 |
| 5678 | 示例DID 2 | 读写 | AA BB CC DD |
| 9ABC | 示例DID 3 | 读写 | 11 22 33 44 55 66 |
| DEFA | 示例DID 4 | 读写 | 00 00 00 00 |
| 0001 | 版本号 | 只读 | V1.0.0 |
| 0002 | 车速 | 只读 | 100 km/h |
| 0003 | 发动机转速 | 只读 | 1000 rpm |
| 0004 | 功能配置字 | 读写 | bit0=1 (功能1开启) |

## 技术栈

- **服务端**：C++11+, 标准socket库
- **客户端**：HTML5, CSS3, JavaScript, WebSocket API
- **桥接服务**：Node.js, ws库
- **构建工具**：CMake
- **数据存储**：JSON格式

## 开发与贡献

### 环境要求

- C++编译器（支持C++11标准）
- Node.js 12.0+（用于WebSocket桥接）
- 浏览器（支持WebSocket API）

### 开发流程

1. 克隆仓库
2. 编译服务端
3. 启动服务端和桥接服务
4. 打开网页客户端进行测试
5. 提交代码变更

## 注意事项

1. 服务端默认监听8888端口，桥接服务默认监听8080端口
2. 首次运行时需要安装Node.js依赖
3. 浏览器可能存在跨域限制，建议使用本地HTTP服务器
4. 服务端支持多客户端连接，但为简化实现，当前使用单线程处理

## 许可证

MIT License

## 联系方式

如有问题或建议，欢迎提出Issue或Pull Request。

## 项目文档

项目详细文档存储在`docs`目录中，包括：

- **project_plan.md** - 项目计划和实施步骤
- **uds_protocol.md** (待创建) - UDS协议实现文档
- **client_usage.md** (待创建) - 客户端使用指南
- **server_deployment.md** (待创建) - 服务端部署指南
- **api_reference.md** (待创建) - API参考文档
- **hsm_simulator.md** (待创建) - HSM模拟器实现文档

请参考这些文档获取更详细的项目信息和使用指南。
