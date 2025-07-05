#pragma once
#include <mysql/mysql.h>
#include <string>
#include <iostream>
#include <ctime>
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
private:
    MYSQL* conn;
    clock_t aliveTime; //记录进入空闲状态后的存活时间
};



