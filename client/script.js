// UDS诊断客户端JavaScript

class UDSClient {
    constructor() {
        this.socket = null;
        this.isConnected = false;
        
        // DOM元素
        this.connectBtn = document.getElementById('connect-btn');
        this.disconnectBtn = document.getElementById('disconnect-btn');
        this.connectStatus = document.getElementById('connection-status');
        this.serverIpInput = document.getElementById('server-ip');
        this.serverPortInput = document.getElementById('server-port');
        this.serviceSelect = document.getElementById('service-select');
        this.didInput = document.getElementById('did-input');
        this.dataInput = document.getElementById('data-input');
        this.sendBtn = document.getElementById('send-btn');
        this.requestMessage = document.getElementById('request-message');
        this.responseMessage = document.getElementById('response-message');
        this.responseStatus = document.getElementById('response-status');
        this.logContainer = document.getElementById('log-container');
        
        // 绑定事件监听器
        this.bindEvents();
    }
    
    bindEvents() {
        // 连接按钮
        this.connectBtn.addEventListener('click', () => this.connect());
        
        // 断开按钮
        this.disconnectBtn.addEventListener('click', () => this.disconnect());
        
        // 发送按钮
        this.sendBtn.addEventListener('click', () => this.sendCommand());
        
        // 服务选择变化
        this.serviceSelect.addEventListener('change', () => this.updateUI());
        
        // 更新UI初始状态
        this.updateUI();
    }
    
    updateUI() {
        // 根据连接状态更新按钮
        this.connectBtn.disabled = this.isConnected;
        this.disconnectBtn.disabled = !this.isConnected;
        this.sendBtn.disabled = !this.isConnected;
        
        // 根据连接状态更新状态显示
        if (this.isConnected) {
            this.connectStatus.textContent = '已连接';
            this.connectStatus.className = 'status connected';
        } else {
            this.connectStatus.textContent = '未连接';
            this.connectStatus.className = 'status';
        }
        
        // 根据服务类型显示/隐藏数据输入框
        const service = this.serviceSelect.value;
        if (service === '2E') {
            // 写入DID需要数据输入
            document.getElementById('data-input-group').style.display = 'flex';
        } else {
            // 读取DID不需要数据输入
            document.getElementById('data-input-group').style.display = 'none';
        }
    }
    
    connect() {
        const ip = this.serverIpInput.value;
        const port = 8080; // WebSocket桥接端口
        
        try {
            // 创建实际的WebSocket连接
            this.log('尝试连接到WebSocket服务器: ' + ip + ':' + port);
            this.socket = new WebSocket(`ws://${ip}:${port}`);
            
            this.socket.onopen = () => {
                this.isConnected = true;
                this.updateUI();
                this.log('成功连接到服务器', 'success');
            };
            
            this.socket.onmessage = (event) => {
                this.handleResponse(event.data);
            };
            
            this.socket.onerror = (error) => {
                this.log('连接错误: ' + error.message, 'error');
            };
            
            this.socket.onclose = () => {
                this.isConnected = false;
                this.updateUI();
                this.log('连接已断开', 'info');
            };
            
        } catch (error) {
            this.log('连接失败: ' + error.message, 'error');
        }
    }
    
    disconnect() {
        try {
            if (this.socket) {
                this.socket.close();
                this.socket = null;
            }
            
            this.isConnected = false;
            this.updateUI();
            this.log('已断开与服务器的连接', 'info');
        } catch (error) {
            this.log('断开连接失败: ' + error.message, 'error');
        }
    }
    
    sendCommand() {
        try {
            const service = this.serviceSelect.value;
            const did = this.didInput.value.trim();
            const data = this.dataInput.value.trim();
            
            // 验证输入
            if (!did) {
                this.log('请输入DID', 'error');
                return;
            }
            
            // 构建UDS请求报文
            const request = this.buildUdsRequest(service, did, data);
            
            // 显示请求报文
            this.requestMessage.textContent = this.formatHex(request);
            
            // 发送请求
            this.log('发送UDS请求: 服务=' + service + ', DID=' + did + ', 数据=' + data);
            
            // 实际发送数据到服务器
            if (this.socket && this.socket.readyState === WebSocket.OPEN) {
                this.socket.send(request);
            } else {
                this.log('WebSocket连接未建立或已关闭', 'error');
            }
            
        } catch (error) {
            this.log('发送命令失败: ' + error.message, 'error');
        }
    }
    
    buildUdsRequest(service, did, data) {
        // 转换服务ID为字节
        const serviceByte = parseInt(service, 16);
        
        // 转换DID为字节（大端序）
        const didValue = parseInt(did, 16);
        const didBytes = [(didValue >> 8) & 0xFF, didValue & 0xFF];
        
        // 转换数据为字节数组
        const dataBytes = [];
        if (data) {
            const dataParts = data.split(/\s+/);
            for (const part of dataParts) {
                if (part) {
                    dataBytes.push(parseInt(part, 16));
                }
            }
        }
        
        // 构建请求字节数组
        const request = [serviceByte];
        request.push(...didBytes);
        request.push(...dataBytes);
        
        return new Uint8Array(request);
    }
    
    handleResponse(responseData) {
        try {
            // 转换响应数据为Uint8Array
            let responseBytes;
            
            // 详细日志记录，便于调试
            this.log('收到响应数据: ' + JSON.stringify(responseData), 'info');
            this.log('响应数据类型: ' + typeof responseData, 'info');
            
            if (responseData instanceof ArrayBuffer) {
                this.log('响应数据是ArrayBuffer', 'info');
                responseBytes = new Uint8Array(responseData);
            } else if (Array.isArray(responseData)) {
                this.log('响应数据是Array', 'info');
                responseBytes = new Uint8Array(responseData);
            } else if (typeof responseData === 'string') {
                // 如果是字符串，直接处理
                this.log('响应数据是字符串', 'info');
                responseBytes = this.hexStringToUint8Array(responseData);
            } else if (responseData instanceof Blob) {
                this.log('响应数据是Blob', 'info');
                // 对于Blob类型，需要转换
                const reader = new FileReader();
                reader.onload = (e) => {
                    const arrayBuffer = e.target.result;
                    responseBytes = new Uint8Array(arrayBuffer);
                    this.responseMessage.textContent = this.formatHex(responseBytes);
                    this.parseResponse(responseBytes);
                };
                reader.readAsArrayBuffer(responseData);
                return; // 异步处理，直接返回
            } else {
                this.log('响应数据是其他类型，尝试直接转换', 'info');
                responseBytes = new Uint8Array(responseData);
            }
            
            // 显示响应报文
            this.responseMessage.textContent = this.formatHex(responseBytes);
            this.log('格式化后的响应报文: ' + this.formatHex(responseBytes), 'info');
            
            // 解析响应
            this.parseResponse(responseBytes);
            
        } catch (error) {
            this.log('处理响应失败: ' + error.message, 'error');
            console.error('响应处理错误:', error);
        }
    }
    
    parseResponse(responseBytes) {
        if (responseBytes.length < 2) {
            this.responseStatus.textContent = '无效响应';
            this.responseStatus.className = 'status error';
            return;
        }
        
        const firstByte = responseBytes[0];
        
        if (firstByte === 0x7F) {
            // 负响应
            const serviceId = responseBytes[1];
            const responseCode = responseBytes[2];
            
            this.responseStatus.textContent = `负响应 - 服务: 0x${serviceId.toString(16).toUpperCase()}, 响应码: 0x${responseCode.toString(16).toUpperCase()}`;
            this.responseStatus.className = 'status error';
            this.log(`收到负响应: 服务=0x${serviceId.toString(16).toUpperCase()}, 响应码=0x${responseCode.toString(16).toUpperCase()}`, 'error');
        } else if (firstByte >= 0x40 && firstByte <= 0x7E) {
            // 正响应
            const serviceId = firstByte - 0x40;
            
            this.responseStatus.textContent = `正响应 - 服务: 0x${serviceId.toString(16).toUpperCase()}`;
            this.responseStatus.className = 'status success';
            this.log(`收到正响应: 服务=0x${serviceId.toString(16).toUpperCase()}`, 'success');
        } else {
            // 未知响应
            this.responseStatus.textContent = '未知响应格式';
            this.responseStatus.className = 'status error';
            this.log('收到未知响应格式', 'error');
        }
    }
    
    // 模拟响应（用于测试）
    simulateResponse(request) {
        // 模拟网络延迟
        setTimeout(() => {
            let response;
            
            // 解析请求
            const serviceId = request[0];
            const did = (request[1] << 8) | request[2];
            
            if (serviceId === 0x22) {
                // 22服务 - 读取DID
                // 模拟正响应
                response = new Uint8Array([0x62, request[1], request[2], 0x01, 0x02, 0x03, 0x04]);
            } else if (serviceId === 0x2E) {
                // 2E服务 - 写入DID
                // 模拟正响应
                response = new Uint8Array([0x6E, request[1], request[2], ...request.slice(3)]);
            } else {
                // 未知服务 - 负响应
                response = new Uint8Array([0x7F, serviceId, 0x11]); // 服务不支持
            }
            
            // 处理响应
            this.handleResponse(response);
        }, 500);
    }
    
    // 工具函数：格式化十六进制数据
    formatHex(data) {
        return Array.from(data)
            .map(byte => byte.toString(16).padStart(2, '0').toUpperCase())
            .join(' ');
    }
    
    // 工具函数：十六进制字符串转Uint8Array
    hexStringToUint8Array(hexString) {
        const hex = hexString.replace(/\s+/g, '');
        const bytes = new Uint8Array(hex.length / 2);
        
        for (let i = 0; i < hex.length; i += 2) {
            bytes[i / 2] = parseInt(hex.substr(i, 2), 16);
        }
        
        return bytes;
    }
    
    // 日志函数
    log(message, type = 'info') {
        const timestamp = new Date().toLocaleTimeString();
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry log-${type}`;
        logEntry.innerHTML = `<span class="log-timestamp">[${timestamp}]</span>${message}`;
        
        this.logContainer.appendChild(logEntry);
        
        // 滚动到底部
        this.logContainer.scrollTop = this.logContainer.scrollHeight;
    }
}

// 页面加载完成后初始化客户端
document.addEventListener('DOMContentLoaded', () => {
    new UDSClient();
});
