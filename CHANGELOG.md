# 更新日志

本文档记录了CHAT项目的所有重要变更。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
并且本项目遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

## [1.0.0] - 2024-12-19

### 新增
- 🎉 **初始版本发布**
- ✨ 基于Muduo网络库的高性能聊天服务器
- 🔐 用户注册、登录、注销功能
- 💬 一对一聊天功能
- 👥 好友管理系统（添加好友、删除好友、好友列表）
- 🏢 群组聊天功能（创建群组、加入群组、群聊）
- 📨 离线消息存储和推送
- 🔄 基于Redis的分布式消息发布/订阅
- 🗄️ MySQL数据库支持，包含连接池优化
- 🧪 性能测试框架
- 📚 完整的项目文档

### 技术特性
- **网络编程**: 基于Muduo库的Reactor模式
- **数据库**: MySQL + 连接池技术
- **缓存**: Redis发布/订阅机制
- **并发**: 多线程安全设计
- **序列化**: JSON消息格式
- **架构**: 模块化设计，职责分离

### 项目结构
```
CHAT/
├── src/           # 源代码
│   ├── server/    # 服务器端代码
│   └── client/    # 客户端代码
├── include/       # 头文件
├── test/          # 测试代码
├── bin/           # 可执行文件
├── docs/          # 文档
└── thirdparty/    # 第三方库
```

### 核心模块
- **ChatService**: 聊天业务逻辑处理
- **UserModel**: 用户数据模型
- **FriendModel**: 好友关系模型
- **GroupModel**: 群组数据模型
- **OfflineMsgModel**: 离线消息模型
- **ConnectionPool**: 数据库连接池
- **Redis**: 分布式消息处理

### 性能指标
- 支持高并发连接
- 数据库连接池优化
- 内存使用优化
- 消息处理延迟优化

### 文档
- 📖 完整的README.md
- 🗺️ 开发路线图 (DEVELOPMENT_ROADMAP.md)
- ❓ 面试问答集 (INTERVIEW_QA.md)
- 📝 回调学习笔记 (CALLBACK_LEARNING_NOTES.md)

---

## 版本说明

### 版本号格式
版本号格式为 `主版本号.次版本号.修订号`，其中：
- **主版本号**: 不兼容的API修改
- **次版本号**: 向下兼容的功能性新增
- **修订号**: 向下兼容的问题修正

### 变更类型
- `新增` - 新功能
- `变更` - 对现有功能的变更
- `弃用` - 即将移除的功能
- `移除` - 已移除的功能
- `修复` - 问题修复
- `安全` - 安全相关的修复

---

## 未来计划

查看 [DEVELOPMENT_ROADMAP.md](DEVELOPMENT_ROADMAP.md) 了解项目的未来发展计划。

## 贡献

欢迎提交Issue和Pull Request来帮助改进项目！

## 许可证

本项目采用 [MIT License](LICENSE) 许可证。