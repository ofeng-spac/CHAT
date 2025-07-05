#include "ConnectionPoolManager.h"
#include "CommonconnectionPool.h"
#include <muduo/base/Logging.h>

ConnectionPoolManager* ConnectionPoolManager::getInstance() {
    static ConnectionPoolManager instance;
    return &instance;
}

bool ConnectionPoolManager::init(const std::string& configFile) {
    if (initialized) {
        return true;
    }
    
    try {
        pool = ConnectionPool::getConnectionPool();
        if (pool != nullptr) {
            initialized = true;
            LOG_INFO << "ConnectionPool initialized successfully!";
            return true;
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to initialize ConnectionPool: " << e.what();
    }
    
    return false;
}

bool ConnectionPoolManager::update(const std::string& sql) {
    if (!initialized) {
        LOG_ERROR << "ConnectionPool not initialized!";
        return false;
    }
    
    auto conn = pool->getConnection();
    if (conn == nullptr) {
        // 注释掉或减少日志输出
        // LOG_ERROR << "Failed to get connection from pool!";
        return false;
    }
    
    bool result = conn->update(sql);
    if (!result) {
        LOG_ERROR << "SQL update failed: " << sql;
    }
    
    return result;
}

MYSQL_RES* ConnectionPoolManager::query(const std::string& sql) {
    if (!initialized) {
        LOG_ERROR << "ConnectionPool not initialized!";
        return nullptr;
    }
    
    auto conn = pool->getConnection();
    if (conn == nullptr) {
        // 在query方法中也做同样的修改
        // 注释掉或减少日志输出
        // LOG_ERROR << "Failed to get connection from pool!";
        return nullptr;
    }
    
    MYSQL_RES* result = conn->query(sql);
    if (result == nullptr) {
        LOG_ERROR << "SQL query failed: " << sql;
    }
    
    return result;
}