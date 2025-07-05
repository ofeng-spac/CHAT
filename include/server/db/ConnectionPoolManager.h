#ifndef CONNECTIONPOOLMANAGER_H
#define CONNECTIONPOOLMANAGER_H

#include <memory>
#include <string>
#include <mysql/mysql.h>

class Connection;
class ConnectionPool;

class ConnectionPoolManager {
public:
    static ConnectionPoolManager* getInstance();
    bool init(const std::string& configFile = "mysql.ini");
    bool update(const std::string& sql);
    MYSQL_RES* query(const std::string& sql);
    
private:
    ConnectionPoolManager() = default;
    ~ConnectionPoolManager() = default;
    ConnectionPoolManager(const ConnectionPoolManager&) = delete;
    ConnectionPoolManager& operator=(const ConnectionPoolManager&) = delete;
    
    ConnectionPool* pool = nullptr;
    bool initialized = false;
};

#endif