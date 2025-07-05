
#include "UserModel.hpp"
#include "db/db.h"
#include "db/ConnectionPoolManager.h"
#include <muduo/base/Logging.h>

// 静态成员初始化
DBConnectionType UserModel::connectionType = DBConnectionType::SINGLE_CONNECTION;

void UserModel::setConnectionType(DBConnectionType type) {
    connectionType = type;
    if (type == DBConnectionType::CONNECTION_POOL) {
        // 初始化连接池
        ConnectionPoolManager::getInstance()->init();
    }
}

// User表的增加操作
bool UserModel::insert(User &user)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    
    if (connectionType == DBConnectionType::CONNECTION_POOL) {
        // 使用连接池
        auto poolManager = ConnectionPoolManager::getInstance();
        if (poolManager->update(sql)) {
            // 注意：连接池方式下获取insert_id需要特殊处理
            // 这里简化处理，实际项目中需要在Connection类中添加getInsertId方法
            user.setId(1); // 临时设置，实际需要获取真实的insert_id
            return true;
        }
    } else {
        // 使用原有的单连接方式
        MySQL mysql;
        if (mysql.connect())
        {
            if (mysql.update(sql))
            {
                // 获取插入成功的用户数据生成的主键id
                user.setId(mysql_insert_id(mysql.getConnection()));
                return true;
            }
        }
    }
    return false;
}

User UserModel::query(int id)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);
    
    if (connectionType == DBConnectionType::CONNECTION_POOL) {
        // 使用连接池
        auto poolManager = ConnectionPoolManager::getInstance();
        MYSQL_RES *res = poolManager->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    } else {
        // 使用原有的单连接方式
        MySQL mysql;
        if (mysql.connect())
        {
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row != nullptr)
                {
                    User user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setPwd(row[2]);
                    user.setState(row[3]);
                    mysql_free_result(res);
                    return user;
                }
            }
        }
    }
    // Return a default user object if no user is found
    return User();
}

bool UserModel::updateState(User user)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());
    
    if (connectionType == DBConnectionType::CONNECTION_POOL) {
        auto poolManager = ConnectionPoolManager::getInstance();
        return poolManager->update(sql);
    } else {
        MySQL mysql;
        if (mysql.connect())
        {
            return mysql.update(sql);
        }
    }
    return false;
}

void UserModel::resetState()
{
    // 1.组装sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";
    
    if (connectionType == DBConnectionType::CONNECTION_POOL) {
        auto poolManager = ConnectionPoolManager::getInstance();
        poolManager->update(sql);
    } else {
        MySQL mysql;
        if (mysql.connect())
        {
            mysql.update(sql);
        }
    }
}

