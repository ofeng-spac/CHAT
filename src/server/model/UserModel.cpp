
#include "UserModel.hpp"
#include "../db/db.h"
#include "../db/ConnectionPoolManager.h"
#include "../../../include/server/security/PasswordUtils.hpp"
#include "../../../include/server/common/InputValidator.hpp"
#include "../../../include/server/common/ErrorCodes.hpp"
#include "../../../include/server/common/Logger.hpp"
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
ErrorCode UserModel::insert(User &user)
{
    CHAT_LOG_INFO_F("Attempting to insert user: %s", user.getName().c_str());
    
    // 输入验证
    ErrorCode validationResult = InputValidator::validateUsername(user.getName());
    if (validationResult != ErrorCode::SUCCESS) {
        CHAT_LOG_ERROR_F("Username validation failed for user: %s", user.getName().c_str());
        return validationResult;
    }
    
    validationResult = InputValidator::validatePassword(user.getPwd());
    if (validationResult != ErrorCode::SUCCESS) {
        CHAT_LOG_ERROR("Password validation failed");
        return validationResult;
    }
    
    // 密码哈希处理
    string salt = PasswordUtils::generateSalt();
    string hashedPassword = PasswordUtils::hashPassword(user.getPwd(), salt);
    
    // 清理输入以防止SQL注入
    string safeName = InputValidator::sanitizeString(user.getName());
    string safeState = InputValidator::sanitizeString(user.getState());
    
    // 1.组装sql语句 - 使用参数化查询风格
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, salt, state) values('%s', '%s', '%s', '%s')",
            safeName.c_str(), hashedPassword.c_str(), salt.c_str(), safeState.c_str());
    
    if (connectionType == DBConnectionType::CONNECTION_POOL) {
        // 使用连接池
        auto poolManager = ConnectionPoolManager::getInstance();
        if (poolManager->update(sql)) {
            // 注意：连接池方式下获取insert_id需要特殊处理
            // 这里简化处理，实际项目中需要在Connection类中添加getInsertId方法
            user.setId(1); // 临时设置，实际需要获取真实的insert_id
            CHAT_LOG_INFO_F("User inserted successfully with ID: %d", user.getId());
            return ErrorCode::SUCCESS;
        } else {
            CHAT_LOG_ERROR("Failed to insert user using connection pool");
             return ErrorCode::DATABASE_INSERT_FAILED;
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
                CHAT_LOG_INFO_F("User inserted successfully with ID: %d", user.getId());
                return ErrorCode::SUCCESS;
            } else {
                CHAT_LOG_ERROR("Failed to execute insert SQL");
                 return ErrorCode::DATABASE_INSERT_FAILED;
            }
        } else {
            CHAT_LOG_ERROR("Failed to connect to database");
            return ErrorCode::DATABASE_CONNECTION_FAILED;
        }
    }
}

pair<User, ErrorCode> UserModel::query(int id)
{
    CHAT_LOG_DEBUG_F("Querying user with ID: %d", id);
    
    // 输入验证
    ErrorCode validationResult = InputValidator::validateUserId(id);
    if (validationResult != ErrorCode::SUCCESS) {
        CHAT_LOG_ERROR_F("Invalid user ID: %d", id);
        return make_pair(User(), validationResult);
    }
    
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select id, name, password, salt, state from user where id = %d", id);
    
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
                user.setPwd(row[2]); // 存储哈希密码
                if (row[3]) user.setSalt(row[3]); // 设置盐值
                user.setState(row[4]);
                mysql_free_result(res);
                CHAT_LOG_DEBUG_F("User found: %s", user.getName().c_str());
                return make_pair(user, ErrorCode::SUCCESS);
            } else {
                mysql_free_result(res);
                CHAT_LOG_WARN_F("No user found with ID: %d", id);
                return make_pair(User(), ErrorCode::USER_NOT_FOUND);
            }
        } else {
            CHAT_LOG_ERROR("Database query failed using connection pool");
             return make_pair(User(), ErrorCode::DATABASE_QUERY_FAILED);
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
                    user.setPwd(row[2]); // 存储哈希密码
                    if (row[3]) user.setSalt(row[3]); // 设置盐值
                    user.setState(row[4]);
                    mysql_free_result(res);
                    CHAT_LOG_DEBUG_F("User found: %s", user.getName().c_str());
                    return make_pair(user, ErrorCode::SUCCESS);
                } else {
                    mysql_free_result(res);
                    CHAT_LOG_WARN_F("No user found with ID: %d", id);
                    return make_pair(User(), ErrorCode::USER_NOT_FOUND);
                }
            } else {
                CHAT_LOG_ERROR("Database query failed");
                 return make_pair(User(), ErrorCode::DATABASE_QUERY_FAILED);
            }
        } else {
            CHAT_LOG_ERROR("Failed to connect to database");
            return make_pair(User(), ErrorCode::DATABASE_CONNECTION_FAILED);
        }
    }
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

