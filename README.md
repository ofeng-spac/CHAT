# CHAT - 高性能聊天服务器

## 项目简介

CHAT是一个基于C++和muduo网络库开发的高性能聊天服务器系统。该项目采用现代C++设计模式，支持用户注册、登录、好友管理、群组聊天等功能，并集成了Redis缓存和MySQL数据库。

## 核心特性

### 🚀 高性能网络架构
- 基于muduo网络库的事件驱动架构
- 支持高并发连接处理
- 异步I/O操作，提升系统吞吐量

### 🔐 安全性保障
- 密码哈希加密存储（SHA-256 + 盐值）
- SQL注入防护
- 输入验证和清理
- 统一错误处理机制

### 💾 数据存储
- MySQL数据库持久化存储
- Redis缓存提升性能
- 数据库连接池管理
- 支持单连接和连接池两种模式

### 📝 完善的日志系统
- 多级别日志记录（DEBUG/INFO/WARN/ERROR/FATAL）
- 日志文件轮转
- 性能监控日志
- 控制台和文件双输出

### 🛡️ 错误处理
- 统一错误码定义
- 标准化错误响应
- 详细的错误日志记录

## 系统架构

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Client App    │    │   Client App    │    │   Client App    │
└─────────┬───────┘    └─────────┬───────┘    └─────────┬───────┘
          │                      │                      │
          └──────────────────────┼──────────────────────┘
                                 │
                    ┌─────────────┴─────────────┐
                    │     Chat Server           │
                    │  (muduo + C++)           │
                    └─────────────┬─────────────┘
                                 │
                    ┌─────────────┴─────────────┐
                    │                           │
          ┌─────────┴─────────┐       ┌─────────┴─────────┐
          │   Redis Cache     │       │   MySQL Database │
          │   (Session/Cache) │       │   (Persistent)    │
          └───────────────────┘       └───────────────────┘
```

## 功能模块

### 用户管理
- 用户注册（密码加密存储）
- 用户登录验证
- 用户状态管理（在线/离线）
- 用户信息查询

### 好友系统
- 添加好友
- 好友列表管理
- 好友状态查询

### 群组功能
- 创建群组
- 加入/退出群组
- 群组消息广播
- 群组成员管理

### 消息系统
- 实时消息传输
- 离线消息存储
- 消息历史记录
- 群组消息处理

## 技术栈

- **编程语言**: C++17
- **网络库**: muduo
- **数据库**: MySQL 8.0+
- **缓存**: Redis 6.0+
- **JSON处理**: nlohmann/json
- **构建工具**: CMake
- **加密**: OpenSSL

## 快速开始

### 环境要求

- Linux操作系统（推荐Ubuntu 20.04+）
- GCC 9.0+ 或 Clang 10.0+
- CMake 3.16+
- MySQL 8.0+
- Redis 6.0+
- muduo网络库

### 安装依赖

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y build-essential cmake git
sudo apt install -y libmysqlclient-dev libssl-dev
sudo apt install -y mysql-server redis-server

# 安装muduo库
git clone https://github.com/chenshuo/muduo.git
cd muduo
./build.sh
sudo ./build.sh install
```

### 数据库配置

1. 启动MySQL服务：
```bash
sudo systemctl start mysql
sudo systemctl enable mysql
```

2. 创建数据库和表：
```sql
-- 连接MySQL
mysql -u root -p

-- 创建数据库
CREATE DATABASE chat_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE chat_db;

-- 创建用户表（更新版本，包含salt字段）
CREATE TABLE user (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    salt VARCHAR(32) NOT NULL,
    state ENUM('online', 'offline') DEFAULT 'offline',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- 创建好友表
CREATE TABLE friend (
    userid INT,
    friendid INT,
    PRIMARY KEY(userid, friendid),
    FOREIGN KEY(userid) REFERENCES user(id) ON DELETE CASCADE,
    FOREIGN KEY(friendid) REFERENCES user(id) ON DELETE CASCADE
);

-- 创建群组表
CREATE TABLE allgroup (
    id INT AUTO_INCREMENT PRIMARY KEY,
    groupname VARCHAR(50) NOT NULL,
    groupdesc VARCHAR(200) DEFAULT '',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建群组用户表
CREATE TABLE groupuser (
    groupid INT,
    userid INT,
    grouprole ENUM('creator', 'admin', 'normal') DEFAULT 'normal',
    PRIMARY KEY(groupid, userid),
    FOREIGN KEY(groupid) REFERENCES allgroup(id) ON DELETE CASCADE,
    FOREIGN KEY(userid) REFERENCES user(id) ON DELETE CASCADE
);

-- 创建离线消息表
CREATE TABLE offlinemessage (
    userid INT,
    message TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(userid) REFERENCES user(id) ON DELETE CASCADE
);
```

3. 创建数据库用户：
```sql
CREATE USER 'chat_user'@'localhost' IDENTIFIED BY 'chat_password';
GRANT ALL PRIVILEGES ON chat_db.* TO 'chat_user'@'localhost';
FLUSH PRIVILEGES;
```

### Redis配置

1. 启动Redis服务：
```bash
sudo systemctl start redis-server
sudo systemctl enable redis-server
```

2. 测试Redis连接：
```bash
redis-cli ping
# 应该返回 PONG
```

### 编译项目

```bash
# 在项目根目录下
mkdir -p build
cd build

# 配置和编译
cmake ..
make -j$(nproc)
```

### 运行服务器

```bash
# 在项目根目录下运行
./bin/ChatServer
```

服务器默认监听端口：6000

## 配置说明

### 数据库配置

在 `include/server/db/db.h` 中修改数据库连接参数：

```cpp
// 数据库配置
#define DB_HOST "localhost"
#define DB_USER "chat_user"
#define DB_PASSWORD "chat_password"
#define DB_NAME "chat_db"
#define DB_PORT 3306
```

### Redis配置

在 `include/server/redis/redis.hpp` 中修改Redis连接参数：

```cpp
// Redis配置
#define REDIS_HOST "127.0.0.1"
#define REDIS_PORT 6379
#define REDIS_PASSWORD ""  // 如果设置了密码
```

### 日志配置

```cpp
// 在main函数中配置日志
LogConfig logConfig;
logConfig.level = LogLevel::INFO;
logConfig.logDir = "./logs";
logConfig.enableConsole = true;
logConfig.enableFile = true;
logConfig.maxFileSize = 10 * 1024 * 1024; // 10MB
logConfig.maxFileCount = 5;

Logger::getInstance().init(logConfig);
```

## API接口

### 用户相关

#### 用户注册
```json
{
    "msgid": 1,
    "name": "username",
    "password": "password"
}
```

#### 用户登录
```json
{
    "msgid": 2,
    "name": "username",
    "password": "password"
}
```

#### 注销登录
```json
{
    "msgid": 3,
    "id": 1001
}
```

### 好友相关

#### 添加好友
```json
{
    "msgid": 4,
    "id": 1001,
    "friendid": 1002
}
```

### 消息相关

#### 一对一聊天
```json
{
    "msgid": 5,
    "id": 1001,
    "from": 1001,
    "to": 1002,
    "msg": "Hello!"
}
```

#### 群聊消息
```json
{
    "msgid": 6,
    "id": 1001,
    "groupid": 1,
    "msg": "Hello group!"
}
```

### 群组相关

#### 创建群组
```json
{
    "msgid": 7,
    "id": 1001,
    "groupname": "技术讨论群",
    "groupdesc": "讨论技术问题的群组"
}
```

#### 加入群组
```json
{
    "msgid": 8,
    "id": 1001,
    "groupid": 1
}
```

## 安全特性

### 密码安全
- SHA-256哈希算法
- 随机盐值生成
- 密码强度验证

### 输入验证
- 用户名格式验证
- 消息内容过滤
- SQL注入防护
- XSS攻击防护

### 会话管理
- 安全的会话令牌
- 会话超时机制
- 多设备登录控制

## 监控和日志

### 日志级别
- **DEBUG**: 详细的调试信息
- **INFO**: 一般信息记录
- **WARN**: 警告信息
- **ERROR**: 错误信息
- **FATAL**: 致命错误

### 性能监控
- 请求响应时间统计
- 数据库查询性能监控
- 内存使用情况跟踪
- 连接数统计

## 故障排除

### 常见问题

1. **编译错误**
   - 检查依赖库是否正确安装
   - 确认C++编译器版本
   - 检查CMake版本

2. **数据库连接失败**
   - 检查MySQL服务状态
   - 验证数据库配置参数
   - 确认用户权限

3. **Redis连接失败**
   - 检查Redis服务状态
   - 验证Redis配置
   - 检查防火墙设置

4. **服务器启动失败**
   - 检查端口是否被占用
   - 查看错误日志
   - 验证配置文件

## 版本历史

### v1.0.0 (当前版本)
- 基础聊天功能实现
- 用户管理系统
- 好友和群组功能
- 安全性改进
- 完善的日志系统
- 错误处理优化

## 许可证

本项目采用MIT许可证，详见 [LICENSE](LICENSE) 文件。