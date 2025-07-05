#pragma once
#include <queue>
#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Connection.h"
/*
实现连接池功能
*/
class ConnectionPool
{
public:
    // 获取连接池对象实例
    static ConnectionPool* getConnectionPool();
    // 给外部提供接口，从连接池中获取一个可用的空闲连接
    shared_ptr<Connection> getConnection();
private:
    // 单例模式
    ConnectionPool();
    bool LoadConfigFile();
    void produceConnectionTask();
    void scannerConnectionTask(); // 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
    string ip; // 数据库连接的ip
    unsigned short port; // 数据库连接的端口
    string username; // 数据库连接的用户名
    string password; // 数据库连接的密码
    string dbname; // 数据库连接的数据库名
    int initSize;   // 连接池初始连接数量
    int maxSize; // 连接池最大连接数量
    int maxIdleTime;    // 最大空闲时间
    int connectionTimeout; // 连接超时时间
    queue<Connection*> connectionQue; //存储mysql连接的队列
    mutex queMutex; // 维护连接池的互斥锁
    atomic_int connectionCnt; // 记录连接池中的连接数量
    condition_variable cv; // 条件变量用于生产者和消费者 两个线程间的通信
};