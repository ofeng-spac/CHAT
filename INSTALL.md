# CHAT 安装指南

本文档提供了CHAT聊天服务器的详细安装和配置指南。

## 系统要求

### 硬件要求
- **CPU**: 2核心以上
- **内存**: 4GB以上
- **存储**: 20GB可用空间
- **网络**: 稳定的网络连接

### 软件要求
- **操作系统**: Linux (Ubuntu 20.04+, CentOS 8+, Debian 10+)
- **编译器**: GCC 9.0+ 或 Clang 10.0+
- **构建工具**: CMake 3.16+
- **数据库**: MySQL 8.0+
- **缓存**: Redis 6.0+
- **网络库**: muduo

## 安装步骤

### 1. 系统准备

#### Ubuntu/Debian 系统

```bash
# 更新系统包
sudo apt update && sudo apt upgrade -y

# 安装基础开发工具
sudo apt install -y build-essential cmake git wget curl
sudo apt install -y pkg-config autoconf automake libtool

# 安装网络和系统库
sudo apt install -y libssl-dev libcurl4-openssl-dev
sudo apt install -y libevent-dev libboost-all-dev
```

#### CentOS/RHEL 系统

```bash
# 更新系统包
sudo yum update -y

# 安装开发工具组
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake3 git wget curl

# 安装依赖库
sudo yum install -y openssl-devel libcurl-devel
sudo yum install -y libevent-devel boost-devel

# 创建cmake软链接（如果需要）
sudo ln -sf /usr/bin/cmake3 /usr/bin/cmake
```

### 2. 安装MySQL 8.0

#### Ubuntu/Debian

```bash
# 安装MySQL服务器
sudo apt install -y mysql-server mysql-client
sudo apt install -y libmysqlclient-dev

# 启动MySQL服务
sudo systemctl start mysql
sudo systemctl enable mysql

# 安全配置（可选）
sudo mysql_secure_installation
```

#### CentOS/RHEL

```bash
# 添加MySQL官方仓库
sudo yum install -y https://dev.mysql.com/get/mysql80-community-release-el8-1.noarch.rpm

# 安装MySQL
sudo yum install -y mysql-server mysql-devel

# 启动MySQL服务
sudo systemctl start mysqld
sudo systemctl enable mysqld

# 获取临时密码
sudo grep 'temporary password' /var/log/mysqld.log

# 安全配置
sudo mysql_secure_installation
```

### 3. 安装Redis 6.0+

#### Ubuntu/Debian

```bash
# 安装Redis
sudo apt install -y redis-server

# 启动Redis服务
sudo systemctl start redis-server
sudo systemctl enable redis-server

# 测试Redis连接
redis-cli ping
```

#### CentOS/RHEL

```bash
# 启用EPEL仓库
sudo yum install -y epel-release

# 安装Redis
sudo yum install -y redis

# 启动Redis服务
sudo systemctl start redis
sudo systemctl enable redis

# 测试Redis连接
redis-cli ping
```

### 4. 安装muduo网络库

```bash
# 克隆muduo源码
cd /tmp
git clone https://github.com/chenshuo/muduo.git
cd muduo

# 编译muduo
./build.sh

# 安装muduo（需要root权限）
sudo ./build.sh install

# 更新动态链接库缓存
sudo ldconfig
```

### 5. 安装其他依赖

#### 安装nlohmann/json库

```bash
# 方法1：使用包管理器（推荐）
# Ubuntu/Debian
sudo apt install -y nlohmann-json3-dev

# CentOS/RHEL（需要EPEL）
sudo yum install -y json-devel

# 方法2：手动安装
cd /tmp
wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
sudo mkdir -p /usr/local/include/nlohmann
sudo cp json.hpp /usr/local/include/nlohmann/
```

#### 安装hiredis（Redis C客户端）

```bash
# Ubuntu/Debian
sudo apt install -y libhiredis-dev

# CentOS/RHEL
sudo yum install -y hiredis-devel

# 或者从源码编译
cd /tmp
git clone https://github.com/redis/hiredis.git
cd hiredis
make
sudo make install
sudo ldconfig
```

## 数据库配置

### 1. 创建数据库和用户

```bash
# 连接到MySQL
mysql -u root -p
```

```sql
-- 创建数据库
CREATE DATABASE chat_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建专用用户
CREATE USER 'chat_user'@'localhost' IDENTIFIED BY 'your_secure_password';
GRANT ALL PRIVILEGES ON chat_db.* TO 'chat_user'@'localhost';
FLUSH PRIVILEGES;

-- 使用数据库
USE chat_db;
```

### 2. 创建数据表

```sql
-- 用户表（包含安全改进）
CREATE TABLE user (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL COMMENT '哈希密码',
    salt VARCHAR(32) NOT NULL COMMENT '密码盐值',
    state ENUM('online', 'offline') DEFAULT 'offline',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_name (name),
    INDEX idx_state (state)
) ENGINE=InnoDB COMMENT='用户信息表';

-- 好友关系表
CREATE TABLE friend (
    userid INT NOT NULL,
    friendid INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (userid, friendid),
    FOREIGN KEY (userid) REFERENCES user(id) ON DELETE CASCADE,
    FOREIGN KEY (friendid) REFERENCES user(id) ON DELETE CASCADE,
    INDEX idx_userid (userid),
    INDEX idx_friendid (friendid)
) ENGINE=InnoDB COMMENT='好友关系表';

-- 群组表
CREATE TABLE allgroup (
    id INT AUTO_INCREMENT PRIMARY KEY,
    groupname VARCHAR(50) NOT NULL,
    groupdesc VARCHAR(200) DEFAULT '',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_groupname (groupname)
) ENGINE=InnoDB COMMENT='群组信息表';

-- 群组成员表
CREATE TABLE groupuser (
    groupid INT NOT NULL,
    userid INT NOT NULL,
    grouprole ENUM('creator', 'admin', 'normal') DEFAULT 'normal',
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (groupid, userid),
    FOREIGN KEY (groupid) REFERENCES allgroup(id) ON DELETE CASCADE,
    FOREIGN KEY (userid) REFERENCES user(id) ON DELETE CASCADE,
    INDEX idx_groupid (groupid),
    INDEX idx_userid (userid)
) ENGINE=InnoDB COMMENT='群组成员表';

-- 离线消息表
CREATE TABLE offlinemessage (
    id INT AUTO_INCREMENT PRIMARY KEY,
    userid INT NOT NULL,
    message TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_read BOOLEAN DEFAULT FALSE,
    FOREIGN KEY (userid) REFERENCES user(id) ON DELETE CASCADE,
    INDEX idx_userid (userid),
    INDEX idx_created_at (created_at),
    INDEX idx_is_read (is_read)
) ENGINE=InnoDB COMMENT='离线消息表';

-- 消息历史表（可选）
CREATE TABLE message_history (
    id INT AUTO_INCREMENT PRIMARY KEY,
    from_userid INT NOT NULL,
    to_userid INT,
    groupid INT,
    message TEXT NOT NULL,
    message_type ENUM('private', 'group') NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (from_userid) REFERENCES user(id) ON DELETE CASCADE,
    FOREIGN KEY (to_userid) REFERENCES user(id) ON DELETE SET NULL,
    FOREIGN KEY (groupid) REFERENCES allgroup(id) ON DELETE SET NULL,
    INDEX idx_from_userid (from_userid),
    INDEX idx_to_userid (to_userid),
    INDEX idx_groupid (groupid),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB COMMENT='消息历史表';
```

### 3. 优化数据库配置

编辑MySQL配置文件 `/etc/mysql/mysql.conf.d/mysqld.cnf`：

```ini
[mysqld]
# 基础配置
port = 3306
bind-address = 127.0.0.1

# 字符集配置
character-set-server = utf8mb4
collation-server = utf8mb4_unicode_ci

# 连接配置
max_connections = 200
max_connect_errors = 10

# 缓冲区配置
innodb_buffer_pool_size = 256M
innodb_log_file_size = 64M
innodb_log_buffer_size = 16M

# 查询缓存
query_cache_type = 1
query_cache_size = 32M

# 慢查询日志
slow_query_log = 1
slow_query_log_file = /var/log/mysql/slow.log
long_query_time = 2

# 错误日志
log_error = /var/log/mysql/error.log
```

重启MySQL服务：
```bash
sudo systemctl restart mysql
```

## Redis配置

### 1. 基础配置

编辑Redis配置文件 `/etc/redis/redis.conf`：

```conf
# 网络配置
bind 127.0.0.1
port 6379
timeout 300

# 内存配置
maxmemory 256mb
maxmemory-policy allkeys-lru

# 持久化配置
save 900 1
save 300 10
save 60 10000

# 日志配置
loglevel notice
logfile /var/log/redis/redis-server.log

# 安全配置（可选）
# requirepass your_redis_password

# 数据库数量
databases 16
```

### 2. 重启Redis服务

```bash
sudo systemctl restart redis-server
# 或者
sudo systemctl restart redis
```

## 编译和安装CHAT

### 1. 获取源码

```bash
# 克隆项目（替换为实际的仓库地址）
git clone https://github.com/your-username/CHAT.git
cd CHAT
```

### 2. 配置编译环境

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# 或者指定安装路径
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
```

### 3. 编译项目

```bash
# 编译（使用所有可用CPU核心）
make -j$(nproc)

# 或者指定核心数
make -j4
```

### 4. 安装（可选）

```bash
# 安装到系统目录
sudo make install

# 更新动态链接库缓存
sudo ldconfig
```

## 配置文件设置

### 1. 数据库配置

编辑 `include/server/db/db.h` 或创建配置文件：

```cpp
// 数据库连接配置
#define DB_HOST "localhost"
#define DB_PORT 3306
#define DB_USER "chat_user"
#define DB_PASSWORD "your_secure_password"
#define DB_NAME "chat_db"
#define DB_CHARSET "utf8mb4"
```

### 2. Redis配置

编辑 `include/server/redis/redis.hpp`：

```cpp
// Redis连接配置
#define REDIS_HOST "127.0.0.1"
#define REDIS_PORT 6379
#define REDIS_PASSWORD ""  // 如果设置了密码
#define REDIS_TIMEOUT 3
#define REDIS_DB 0
```

### 3. 服务器配置

创建配置文件 `config/server.conf`：

```ini
[server]
port = 6000
thread_num = 4
max_connections = 1000

[database]
host = localhost
port = 3306
user = chat_user
password = your_secure_password
database = chat_db
max_connections = 10

[redis]
host = 127.0.0.1
port = 6379
password = 
timeout = 3

[log]
level = INFO
dir = ./logs
max_file_size = 10485760
max_file_count = 5
```

## 运行和测试

### 1. 创建必要目录

```bash
# 在项目根目录下
mkdir -p logs
mkdir -p config
```

### 2. 启动服务器

```bash
# 在项目根目录下
./bin/ChatServer

# 或者指定配置文件
./bin/ChatServer --config=config/server.conf

# 后台运行
nohup ./bin/ChatServer > logs/server.log 2>&1 &
```

### 3. 测试连接

```bash
# 测试服务器端口
telnet localhost 6000

# 或者使用netcat
nc localhost 6000

# 检查进程
ps aux | grep ChatServer

# 检查端口占用
netstat -tlnp | grep 6000
```

### 4. 运行客户端（如果有）

```bash
./bin/ChatClient
```

## 系统服务配置（可选）

### 1. 创建systemd服务文件

创建 `/etc/systemd/system/chat-server.service`：

```ini
[Unit]
Description=CHAT Server
After=network.target mysql.service redis.service
Requires=mysql.service redis.service

[Service]
Type=simple
User=chat
Group=chat
WorkingDirectory=/opt/chat
ExecStart=/opt/chat/bin/ChatServer
ExecReload=/bin/kill -HUP $MAINPID
Restart=always
RestartSec=5

# 环境变量
Environment=LD_LIBRARY_PATH=/usr/local/lib

# 日志
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

### 2. 创建专用用户

```bash
# 创建系统用户
sudo useradd -r -s /bin/false chat

# 创建安装目录
sudo mkdir -p /opt/chat
sudo cp -r . /opt/chat/
sudo chown -R chat:chat /opt/chat
```

### 3. 启用服务

```bash
# 重新加载systemd配置
sudo systemctl daemon-reload

# 启用服务
sudo systemctl enable chat-server

# 启动服务
sudo systemctl start chat-server

# 查看状态
sudo systemctl status chat-server

# 查看日志
sudo journalctl -u chat-server -f
```

## 防火墙配置

### Ubuntu/Debian (ufw)

```bash
# 允许服务器端口
sudo ufw allow 6000/tcp

# 允许MySQL（如果需要远程访问）
sudo ufw allow 3306/tcp

# 允许Redis（如果需要远程访问）
sudo ufw allow 6379/tcp

# 启用防火墙
sudo ufw enable
```

### CentOS/RHEL (firewalld)

```bash
# 允许服务器端口
sudo firewall-cmd --permanent --add-port=6000/tcp

# 允许MySQL
sudo firewall-cmd --permanent --add-service=mysql

# 重新加载防火墙
sudo firewall-cmd --reload
```

## 故障排除

### 1. 编译问题

```bash
# 检查依赖库
pkg-config --list-all | grep -E "mysql|hiredis"

# 检查muduo安装
ls /usr/local/lib/libmuduo*

# 检查头文件
ls /usr/local/include/muduo/

# 清理构建缓存
rm -rf build/*
```

### 2. 运行时问题

```bash
# 检查动态链接库
ldd bin/ChatServer

# 检查服务状态
sudo systemctl status mysql redis-server

# 检查日志
tail -f logs/chat_server.log
tail -f /var/log/mysql/error.log
tail -f /var/log/redis/redis-server.log
```

### 3. 网络问题

```bash
# 检查端口监听
sudo netstat -tlnp | grep -E "3306|6379|6000"

# 测试数据库连接
mysql -h localhost -u chat_user -p chat_db

# 测试Redis连接
redis-cli -h 127.0.0.1 -p 6379 ping
```

### 4. 权限问题

```bash
# 检查文件权限
ls -la bin/ChatServer

# 检查目录权限
ls -la logs/

# 修复权限
chmod +x bin/ChatServer
chmod 755 logs/
```

## 性能优化建议

### 1. 系统级优化

```bash
# 增加文件描述符限制
echo "* soft nofile 65536" | sudo tee -a /etc/security/limits.conf
echo "* hard nofile 65536" | sudo tee -a /etc/security/limits.conf

# 优化网络参数
echo "net.core.somaxconn = 1024" | sudo tee -a /etc/sysctl.conf
echo "net.ipv4.tcp_max_syn_backlog = 1024" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

### 2. 数据库优化

```sql
-- 创建索引
CREATE INDEX idx_user_name_state ON user(name, state);
CREATE INDEX idx_message_userid_created ON offlinemessage(userid, created_at);

-- 分析表
ANALYZE TABLE user, friend, allgroup, groupuser, offlinemessage;
```

### 3. Redis优化

```conf
# 在redis.conf中添加
tcp-keepalive 300
tcp-backlog 511
maxclients 10000
```

## 监控和维护

### 1. 日志轮转

创建 `/etc/logrotate.d/chat-server`：

```
/opt/chat/logs/*.log {
    daily
    missingok
    rotate 30
    compress
    delaycompress
    notifempty
    copytruncate
    create 644 chat chat
}
```

### 2. 备份脚本

创建数据库备份脚本：

```bash
#!/bin/bash
# backup_db.sh

DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_DIR="/opt/chat/backups"
DB_NAME="chat_db"
DB_USER="chat_user"
DB_PASS="your_secure_password"

mkdir -p $BACKUP_DIR

# 备份数据库
mysqldump -u $DB_USER -p$DB_PASS $DB_NAME > $BACKUP_DIR/chat_db_$DATE.sql

# 压缩备份文件
gzip $BACKUP_DIR/chat_db_$DATE.sql

# 删除7天前的备份
find $BACKUP_DIR -name "*.sql.gz" -mtime +7 -delete

echo "Database backup completed: chat_db_$DATE.sql.gz"
```

### 3. 健康检查脚本

```bash
#!/bin/bash
# health_check.sh

# 检查服务器进程
if ! pgrep -f ChatServer > /dev/null; then
    echo "ChatServer is not running!"
    # 可以添加重启逻辑
    # systemctl restart chat-server
fi

# 检查数据库连接
if ! mysql -u chat_user -pyour_secure_password -e "SELECT 1" chat_db > /dev/null 2>&1; then
    echo "Database connection failed!"
fi

# 检查Redis连接
if ! redis-cli ping > /dev/null 2>&1; then
    echo "Redis connection failed!"
fi

# 检查端口
if ! netstat -tlnp | grep :6000 > /dev/null; then
    echo "Server port 6000 is not listening!"
fi
```

## 安全建议

1. **定期更新系统和依赖库**
2. **使用强密码**
3. **限制数据库和Redis的网络访问**
4. **启用防火墙**
5. **定期备份数据**
6. **监控系统日志**
7. **使用SSL/TLS加密通信**（后续版本）

## 总结

完成以上步骤后，CHAT聊天服务器应该已经成功安装并运行。如果遇到问题，请检查：

1. 所有依赖库是否正确安装
2. 数据库和Redis服务是否正常运行
3. 配置文件是否正确
4. 防火墙和网络设置
5. 文件权限和用户权限

如需帮助，请查看项目文档或提交Issue。