#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"

enum class DBConnectionType {
    SINGLE_CONNECTION,  // 原有的单连接方式
    CONNECTION_POOL     // 连接池方式
};

class UserModel{
public:
    // 设置数据库连接类型
    static void setConnectionType(DBConnectionType type);
    
    // 用户注册
    bool insert(User &user);
    // 根据用户号码查询用户信息
    User query(int id);
    // 更新用户的状态信息
    bool updateState(User user);
    // 重置用户的状态信息
    void resetState();
    
private:
    static DBConnectionType connectionType;
};

#endif