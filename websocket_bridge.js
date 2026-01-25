const WebSocket = require('ws');
const net = require('net');

// WebSocket服务器配置
const wsPort = 8080;

// TCP服务器配置
const tcpHost = '127.0.0.1';
const tcpPort = 8888;

// 创建WebSocket服务器
const wss = new WebSocket.Server({ port: wsPort });

console.log(`WebSocket-TCP桥接服务启动，监听WebSocket端口: ${wsPort}，转发到TCP: ${tcpHost}:${tcpPort}`);

// 处理WebSocket连接
wss.on('connection', (ws) => {
    console.log('新的WebSocket客户端连接');
    
    // 创建TCP连接
    const tcpClient = net.createConnection({ host: tcpHost, port: tcpPort }, () => {
        console.log(`TCP连接已建立: ${tcpHost}:${tcpPort}`);
    });
    
    // 处理TCP数据
    tcpClient.on('data', (data) => {
        console.log(`从TCP接收到数据: ${data.toString('hex')}`);
        // 将TCP数据转发到WebSocket客户端
        ws.send(data);
    });
    
    // 处理TCP连接关闭
    tcpClient.on('close', () => {
        console.log('TCP连接已关闭');
        ws.close();
    });
    
    // 处理TCP连接错误
    tcpClient.on('error', (err) => {
        console.error(`TCP连接错误: ${err.message}`);
        ws.close();
    });
    
    // 处理WebSocket消息
    ws.on('message', (message) => {
        console.log(`从WebSocket接收到消息: ${Buffer.isBuffer(message) ? message.toString('hex') : message}`);
        // 将WebSocket消息转发到TCP服务器
        tcpClient.write(message);
    });
    
    // 处理WebSocket连接关闭
    ws.on('close', () => {
        console.log('WebSocket客户端连接已关闭');
        tcpClient.end();
    });
    
    // 处理WebSocket连接错误
    ws.on('error', (err) => {
        console.error(`WebSocket错误: ${err.message}`);
        tcpClient.end();
    });
});

// 处理WebSocket服务器错误
wss.on('error', (err) => {
    console.error(`WebSocket服务器错误: ${err.message}`);
});
