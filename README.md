# CHAT - 基于C++的多人即时通讯系统

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-11-blue.svg)](https://isocpp.org/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

## 📖 项目简介

CHAT是一个基于C++开发的高性能多人即时通讯系统，采用现代C++技术栈，支持用户注册登录、好友管理、一对一聊天、群组聊天等核心功能。项目展示了网络编程、数据库设计、缓存应用等后端核心技术的综合运用。

## ✨ 核心特性

- 🚀 **高性能网络架构**：基于Muduo网络库的Reactor模式，支持高并发连接
- 💾 **完整数据存储**：MySQL持久化存储 + Redis缓存，保证数据可靠性
- 🔄 **灵活连接池**：支持单连接和连接池两种模式，适应不同性能需求
- 📨 **离线消息推送**：Redis发布订阅机制，支持跨服务器消息分发
- 🎯 **回调驱动架构**：优雅的消息分发机制，易于扩展新功能
- 📊 **性能测试框架**：内置多线程并发测试，量化评估系统性能

## 🛠️ 技术栈

| 技术领域 | 技术选型 | 说明 |
|---------|---------|------|
| **网络编程** | Muduo | 基于Reactor模式的高性能网络库 |
| **数据存储** | MySQL 8.0+ | 关系型数据库，存储用户数据和消息 |
| **缓存系统** | Redis 6.0+ | 内存数据库，缓存离线消息和会话状态 |
| **消息格式** | JSON | 轻量级数据交换格式 |
| **构建系统** | CMake 3.10+ | 跨平台构建工具 |
| **编程语言** | C++11 | 现代C++特性，智能指针、lambda等 |

## 🏗️ 系统架构

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Chat Client   │    │   Chat Client   │    │   Chat Client   │
└─────────┬───────┘    └─────────┬───────┘    └─────────┬───────┘
          │                      │                      │
          └──────────────────────┼──────────────────────┘
                                 │
                    ┌─────────────┴─────────────┐
                    │      Chat Server          │
                    │   (Muduo + Reactor)       │
                    └─────────────┬─────────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    │     Business Logic        │
                    │   (Message Handlers)      │
                    └─────────────┬─────────────┘
                                  │
        ┌─────────────────────────┼─────────────────────────┐
        │                         │                         │
┌───────▼────────┐    ┌──────────▼──────────┐    ┌────────▼────────┐
│     MySQL      │    │   Connection Pool    │    │     Redis       │
│  (User Data)   │    │   (Performance)     │    │ (Cache & PubSub) │
└────────────────┘    └─────────────────────┘    └─────────────────┘
```

## 📋 功能模块

### 用户管理
- ✅ 用户注册与登录
- ✅ 用户状态管理（在线/离线）
- ✅ 密码验证机制

### 好友系统
- ✅ 添加/删除好友
- ✅ 好友列表管理
- ✅ 好友状态查看

### 消息系统
- ✅ 一对一实时聊天
- ✅ 群组创建与管理
- ✅ 群组聊天功能
- ✅ 离线消息存储与推送

### 性能优化
- ✅ 数据库连接池
- ✅ Redis缓存机制
- ✅ 异步消息处理
- ✅ 多线程性能测试

## 🚀 快速开始

### 环境要求

- **操作系统**：Linux (Ubuntu 18.04+, CentOS 7+)
- **编译器**：GCC 7.0+ 或 Clang 6.0+
- **CMake**：3.10+
- **MySQL**：8.0+
- **Redis**：6.0+
- **依赖库**：Muduo, nlohmann/json

### 安装依赖

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake git
sudo apt install mysql-server mysql-client libmysqlclient-dev
sudo apt install redis-server libhiredis-dev

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install cmake git
sudo yum install mysql-server mysql-devel
sudo yum install redis hiredis-devel
```

### 编译项目

```bash
# 克隆项目
git clone https://github.com/yourusername/CHAT.git
cd CHAT

# 创建构建目录
mkdir build && cd build

# 配置和编译
cmake ..
make -j$(nproc)

# 或者直接在项目根目录
cmake .
make
```

### 数据库配置

1. **创建数据库**
```sql
CREATE DATABASE chat_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE chat_db;

-- 用户表
CREATE TABLE user (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    state ENUM('online', 'offline') DEFAULT 'offline',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_name (name)
);

-- 好友关系表
CREATE TABLE friend (
    userid INT,
    friendid INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (userid, friendid),
    FOREIGN KEY (userid) REFERENCES user(id),
    FOREIGN KEY (friendid) REFERENCES user(id)
);

-- 群组表
CREATE TABLE allgroup (
    id INT PRIMARY KEY AUTO_INCREMENT,
    groupname VARCHAR(50) NOT NULL,
    groupdesc VARCHAR(200),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 群组用户表
CREATE TABLE groupuser (
    groupid INT,
    userid INT,
    grouprole ENUM('creator', 'admin', 'normal') DEFAULT 'normal',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (groupid, userid),
    FOREIGN KEY (groupid) REFERENCES allgroup(id),
    FOREIGN KEY (userid) REFERENCES user(id)
);

-- 离线消息表
CREATE TABLE offlinemessage (
    userid INT,
    message TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_userid (userid),
    FOREIGN KEY (userid) REFERENCES user(id)
);
```

2. **配置数据库连接**

编辑 `mysql.ini` 文件：
```ini
[mysql]
ip=127.0.0.1
port=3306
username=your_username
password=your_password
dbname=chat_db
```

### 启动服务

```bash
# 启动Redis服务
sudo systemctl start redis

# 启动MySQL服务
sudo systemctl start mysql

# 启动聊天服务器
./bin/ChatServer

# 启动客户端（新终端）
./bin/ChatClient
```

## 📊 性能测试

项目内置了性能测试框架，可以评估系统在不同负载下的表现：

```bash
# 运行性能测试
./bin/performance_test

# 测试结果示例
# 并发用户数: 1000
# 消息总数: 10000
# 测试时长: 5.2秒
# QPS: 1923
# 成功率: 99.8%
```

## 📁 项目结构

```
CHAT/
├── CMakeLists.txt              # 主构建配置
├── README.md                   # 项目说明文档
├── DEVELOPMENT_ROADMAP.md      # 开发路线图
├── INTERVIEW_QA.md            # 面试问答文档
├── mysql.ini                   # 数据库配置文件
├── include/                    # 头文件目录
│   ├── public.hpp             # 公共头文件
│   └── server/                # 服务器头文件
│       ├── chatserver.hpp     # 服务器主类
│       ├── chatservice.hpp    # 业务逻辑服务
│       ├── db/                # 数据库相关
│       ├── model/             # 数据模型
│       └── redis/             # Redis相关
├── src/                       # 源代码目录
│   ├── client/                # 客户端代码
│   │   └── main.cpp          # 客户端主程序
│   └── server/                # 服务器代码
│       ├── main.cpp          # 服务器主程序
│       ├── chatserver.cpp    # 服务器实现
│       ├── chatservice.cpp   # 业务逻辑实现
│       ├── db/               # 数据库实现
│       ├── model/            # 数据模型实现
│       └── redis/            # Redis实现
├── test/                      # 测试代码
│   └── performance_test.cpp   # 性能测试
├── thirdparty/               # 第三方库
│   └── json.hpp              # JSON库
├── bin/                      # 可执行文件目录
│   ├── ChatServer            # 服务器程序
│   ├── ChatClient            # 客户端程序
│   └── performance_test      # 性能测试程序
└── build/                    # 构建临时文件
```

## 🎯 使用示例

### 客户端操作示例

```bash
# 启动客户端后的操作流程

# 1. 注册新用户
register
请输入用户名: alice
请输入密码: 123456
注册成功！

# 2. 登录
login
请输入用户名: alice
请输入密码: 123456
登录成功！

# 3. 添加好友
addfriend
请输入好友用户名: bob
添加好友成功！

# 4. 一对一聊天
chat
请输入聊天对象用户名: bob
请输入聊天内容: Hello Bob!
消息发送成功！

# 5. 创建群组
creategroup
请输入群组名称: 技术交流群
请输入群组描述: C++技术交流
群组创建成功！群组ID: 1

# 6. 群组聊天
groupchat
请输入群组ID: 1
请输入聊天内容: 大家好！
群消息发送成功！
```

## 🔧 配置说明

### 数据库连接池配置

在 `ConnectionPoolManager` 中可以调整连接池参数：

```cpp
// 最大连接数
static const int MAX_CONNECTION_COUNT = 10;

// 连接超时时间（秒）
static const int CONNECTION_TIMEOUT = 30;

// 连接空闲时间（秒）
static const int IDLE_TIMEOUT = 300;
```

### Redis配置

```cpp
// Redis服务器配置
static const string REDIS_HOST = "127.0.0.1";
static const int REDIS_PORT = 6379;
static const int REDIS_TIMEOUT = 3;
```

## 🚧 开发路线图

查看 [DEVELOPMENT_ROADMAP.md](DEVELOPMENT_ROADMAP.md) 了解项目的详细开发计划和改进方向。

### 近期计划 (v1.1)
- [ ] 密码加密存储
- [ ] SQL注入防护
- [ ] 完善错误处理机制
- [ ] 添加日志系统

### 中期计划 (v1.2)
- [ ] 支持文件传输
- [ ] 消息加密传输
- [ ] 用户权限管理
- [ ] 群组管理员功能

### 长期计划 (v2.0)
- [ ] 微服务架构重构
- [ ] 分布式部署支持
- [ ] 负载均衡
- [ ] 监控和告警系统

## 🤝 贡献指南

欢迎贡献代码！请遵循以下步骤：

1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

### 代码规范

- 遵循 Google C++ 编码规范
- 使用有意义的变量和函数名
- 添加必要的注释
- 确保代码通过所有测试

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 👥 作者

- **开发者** - *初始工作* - [YourGitHub](https://github.com/yourusername)

## 🙏 致谢

- [Muduo](https://github.com/chenshuo/muduo) - 高性能网络库
- [nlohmann/json](https://github.com/nlohmann/json) - 现代C++ JSON库
- [MySQL](https://www.mysql.com/) - 关系型数据库
- [Redis](https://redis.io/) - 内存数据库

## 📞 联系方式

如有问题或建议，请通过以下方式联系：

- 邮箱：your.email@example.com
- GitHub Issues：[项目Issues页面](https://github.com/yourusername/CHAT/issues)

---

⭐ 如果这个项目对你有帮助，请给它一个星标！