
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include "pch.h"
#include "CommonconnectionPool.h"
using namespace std;

// 添加 LOG 宏定义（如果没有在其他地方定义）
#ifndef LOG
#define LOG(msg) std::cout << msg << std::endl
#endif

/*
实现连接池功能
*/
//线程安全的懒汉模式
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool connPool; //lock
    return &connPool;
}
bool ConnectionPool::LoadConfigFile()
{
    FILE* pf = fopen("mysql.ini", "r");
    if(pf == nullptr)
    {
        LOG("mysql.ini file is not exist!");
        return false;
    }
    while(!feof(pf))
    {
        char line[1024] = {0};
        fgets(line, 1024, pf);
        string str = line;
        int idx = str.find("=", 0);
        if(idx == -1)
        {
            continue;
        }
        int endidx = str.find("\n", idx);
        string key = str.substr(0, idx);
        string val = str.substr(idx+1, endidx-idx-1);
        if(key == "ip")
        {
            ip = val;
        }
        else if(key == "port")
        {
            port = atoi(val.c_str());
        }
        else if(key == "user")
        {
            username = val;
        }
        else if(key == "password")
        {
            password = val;
        }
        else if(key == "dbname")
        {
            dbname = val;
        }
        else if(key == "initsize")
        {
            initSize = atoi(val.c_str());
        }
        else if(key == "maxsize")
        {
            maxSize = atoi(val.c_str());
        }
        else if(key == "maxIdletime")
        {
            maxIdleTime = atoi(val.c_str());
        }
        else if(key == "connectiontimeout")
        {
            connectionTimeout = atoi(val.c_str());
        }   
    }
    fclose(pf);
    return true;
}
ConnectionPool::ConnectionPool()//线程池构造
{
    //加载配置文件
    if(!LoadConfigFile())
    {
        LOG("LoadConfigFile() fail!");
        return;
    }
    //创建初始数量的连接
    for(int i = 0; i < initSize; i++)
    {
        Connection* p = new Connection();
        p->connect(ip, port, username, password, dbname);
        p->refreshAliveTime();//刷新一下开始空闲的起始时间
        connectionQue.push(p);
        connectionCnt++;
    }
    //启动一个新的线程，作为连接的生产者
    thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();
    //启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
    thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();//设置为守护线程
    
}
//运行在独立的线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTask()
{
    while(true)
    {
        unique_lock<mutex> lock(queMutex);
        while(!connectionQue.empty())
        {
            cv.wait(lock);//队列不空,此处线程进入阻塞状态

        }
        if(connectionCnt < maxSize)//连接数量没有到达上限，创建新连接
        {
            Connection* p = new Connection();
            p->connect(ip, port, username, password, dbname);
            p->refreshAliveTime();//刷新一下开始空闲的起始时间
            connectionQue.push(p);
            connectionCnt++;
        }
        cv.notify_all();//通知消费者线程，可以消费连接了
    }   
}
shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(queMutex);
    static int timeout_count = 0;  // 添加静态计数器
    while(connectionQue.empty())
    {
        if(cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(connectionTimeout)))
        {
            if(connectionQue.empty())
            {
                timeout_count++;
                // 只在每100次超时时输出一次日志，或者完全注释掉
                if (timeout_count % 100 == 0) {
                    LOG("连接超时次数: " + to_string(timeout_count));
                }
                return nullptr;
            }
        }
    }
    shared_ptr<Connection> sp(connectionQue.front(),
        [&](Connection* pcon)
        {
            // 这里是连接的归还逻辑
            lock_guard<mutex> lock(queMutex);
            pcon->refreshAliveTime();//刷新一下开始空闲的起始时间
            connectionQue.push(pcon);
        }
    );
    connectionQue.pop();
    cv.notify_all();//消费完连接后，通知生产者线程，可以生产连接了
    return sp;
}
void ConnectionPool::scannerConnectionTask()
{
    while(true)
    {
        // 通过算法扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
        this_thread::sleep_for(chrono::seconds(maxIdleTime));//定时扫描
        //扫描整个队列，释放多余的连接
        lock_guard<mutex> lock(queMutex);
        while(connectionCnt > initSize)
        {
            Connection* p = connectionQue.front();
            if(p->getAliveTime() >= (maxIdleTime*1000))
            {
                connectionQue.pop();
                connectionCnt--;
                delete p;//释放连接
            }
            else
            {
                break;//队头的连接没有超过maxIdleTime,其他连接肯定没有
            }
        }
    }   
}