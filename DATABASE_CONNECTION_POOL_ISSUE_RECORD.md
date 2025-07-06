# 数据库连接池问题记录与解决方案

## 问题描述

在运行CHAT聊天服务器的性能测试时，遇到了数据库连接池无法正常工作的问题：

- 所有数据库操作都失败（成功数为0）
- 日志显示大量"连接超时次数"信息
- 连接池初始化显示成功，但实际无法获取可用连接

## 问题诊断过程

### 1. 初步检查

- **MySQL服务状态**：通过 `mysql -u root -p123456 -e 'SELECT 1'` 确认MySQL服务正常运行
- **数据库存在性**：确认 `chat` 数据库存在且包含正确的表结构和测试数据
- **MySQL连接数限制**：检查 `max_connections = 151`，连接数充足

### 2. 连接池初始化分析

通过添加调试日志发现关键问题：

```
ConnectionPool constructor started
Config loaded successfully. initSize=0, maxSize=0
```

**问题根因**：配置文件解析失败，导致 `initSize` 和 `maxSize` 都为0，连接池无法创建任何连接。

### 3. 配置文件解析问题

**文件内容**（`mysql.ini`）：
```ini
ip = 127.0.0.1
port = 3306
user = root
password = 123456
dbname = chat
initsize = 10
maxsize = 1024
maxidletime = 60
connnectiontimeout = 1000
```

**解析代码问题**：
```cpp
string key = str.substr(0, idx);
string val = str.substr(idx+1, endidx-idx-1);
```

由于配置文件格式为 `initsize = 10`（键值间有空格），但解析代码没有去除前后空格，导致：
- 实际解析的key为 `"initsize "` （包含尾部空格）
- 条件判断 `key == "initsize"` 失败
- `initSize` 保持默认值0

## 解决方案

### 1. 修复配置文件解析

在 `CommonconnectionPool.cpp` 的 `LoadConfigFile()` 方法中添加字符串trim功能：

```cpp
string key = str.substr(0, idx);
string val = str.substr(idx+1, endidx-idx-1);

// 去除key和val前后的空格
key.erase(0, key.find_first_not_of(" \t"));
key.erase(key.find_last_not_of(" \t") + 1);
val.erase(0, val.find_first_not_of(" \t"));
val.erase(val.find_last_not_of(" \t") + 1);
```

### 2. 修正数据库配置

将 `mysql.ini` 中的数据库名从 `test` 修改为 `chat`：
```ini
dbname = chat
```

### 3. 增强错误处理

#### 3.1 连接池构造函数
```cpp
// 创建初始数量的连接
for(int i = 0; i < initSize; i++)
{
    Connection* p = new Connection();
    if (p->connect(ip, port, username, password, dbname)) {
        p->refreshAliveTime();
        connectionQue.push(p);
        connectionCnt++;
    } else {
        LOG("Failed to create initial connection " + to_string(i));
        delete p;
    }
}
```

#### 3.2 生产者线程
```cpp
if(connectionCnt < maxSize)
{
    Connection* p = new Connection();
    if (p->connect(ip, port, username, password, dbname)) {
        p->refreshAliveTime();
        connectionQue.push(p);
        connectionCnt++;
    } else {
        LOG("Failed to create new connection in producer");
        delete p;
    }
}
```

#### 3.3 Connection类错误输出
```cpp
bool Connection::connect(string ip,unsigned short port,string user,string password,string dbname)
{
    MYSQL *p = mysql_real_connect(this->conn,ip.c_str(),user.c_str(),password.c_str(),dbname.c_str(),port,nullptr,0);
    if (p == nullptr) {
        cout << "MySQL connection failed: " << mysql_error(this->conn) << endl;
        cout << "Connection params: ip=" << ip << ", port=" << port << ", user=" << user << ", dbname=" << dbname << endl;
    }
    return p!=nullptr;
}
```

## 修复结果

修复后的测试结果：
```
ConnectionPool constructor started
Config loaded successfully. initSize=10, maxSize=1024
Connection() [x10]
20250705 11:51:42.613017Z 3552750 INFO ConnectionPool initialized successfully!
```

- 配置文件正确解析（initSize=10, maxSize=1024）
- 成功创建10个初始连接
- 连接池正常初始化
- 能够执行数据库查询操作

## 经验总结

### 1. 配置文件解析的健壮性
- 始终要考虑配置文件格式的容错性
- 对键值对进行trim处理，去除前后空格
- 添加配置验证和错误提示

### 2. 调试策略
- 逐层添加日志，从外到内定位问题
- 先验证外部依赖（MySQL服务、数据库、表结构）
- 再检查内部逻辑（配置解析、连接创建）

### 3. 错误处理最佳实践
- 在关键步骤添加返回值检查
- 提供详细的错误信息输出
- 避免静默失败，确保问题可追踪

### 4. 代码改进建议
- 使用更健壮的配置文件解析库（如JSON、YAML）
- 添加配置文件格式验证
- 实现连接池健康检查机制
- 考虑使用现代C++的智能指针管理资源

## 相关文件

- `src/server/db/CommonconnectionPool.cpp` - 连接池实现
- `src/server/db/Connection.cpp` - 数据库连接封装
- `mysql.ini` - 数据库配置文件
- `src/server/db/ConnectionPoolManager.cpp` - 连接池管理器

---

**记录时间**：2025年1月7日  
**问题状态**：已解决  
**影响范围**：数据库连接池初始化和连接获取