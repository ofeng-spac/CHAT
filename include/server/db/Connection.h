#pragma once
#include <mysql/mysql.h>
#include <string>
#include <iostream>
#include <ctime>
#include <mutex>
#include <atomic>
using namespace std;

class Connection
{
public:
    Connection();
    ~Connection();
    bool connect(string ip, unsigned short port, string username, string password, string dbname);
    bool update(string sql);
    MYSQL_RES* query(string sql);
    void refreshAliveTime()//刷新起始空闲时间
    {
        aliveTime = clock();
    } 
    clock_t getAliveTime()//返回存活时间
    {
        return clock() - aliveTime;
    }
    // 检查连接是否有效
    bool isValid();
    // 获取原始MySQL连接指针（谨慎使用）
    MYSQL* getConnection();
    
private:
    MYSQL* conn;
    clock_t aliveTime; //记录进入空闲状态后的存活时间
    mutable std::mutex conn_mutex; // 连接互斥锁
    std::atomic<bool> in_use{false}; // 连接使用状态
};



