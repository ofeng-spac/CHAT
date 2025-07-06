#ifndef SECUREUSERMODEL_HPP
#define SECUREUSERMODEL_HPP

#include "user.hpp"
#include "../common/ErrorCodes.hpp"
#include "../db/SecureDB.hpp"
#include <utility>
#include <memory>

/**
 * 安全用户模型类
 * 使用预编译语句防止SQL注入攻击
 */
class SecureUserModel {
public:
    SecureUserModel();
    ~SecureUserModel();
    
    /**
     * 用户注册
     * @param user 用户对象
     * @return 错误码
     */
    ErrorCode insert(User& user);
    
    /**
     * 根据用户ID查询用户信息
     * @param id 用户ID
     * @return 用户对象和错误码的pair
     */
    std::pair<User, ErrorCode> queryById(int id);
    
    /**
     * 根据用户名查询用户信息
     * @param name 用户名
     * @return 用户对象和错误码的pair
     */
    std::pair<User, ErrorCode> queryByName(const std::string& name);
    
    /**
     * 用户登录验证
     * @param name 用户名
     * @param password 密码
     * @return 用户对象和错误码的pair
     */
    std::pair<User, ErrorCode> login(const std::string& name, const std::string& password);
    
    /**
     * 更新用户状态
     * @param userId 用户ID
     * @param state 新状态
     * @return 错误码
     */
    ErrorCode updateState(int userId, const std::string& state);
    
    /**
     * 重置所有用户状态为offline
     * @return 错误码
     */
    ErrorCode resetAllStates();
    
    /**
     * 检查用户名是否已存在
     * @param name 用户名
     * @return 是否存在
     */
    bool isUsernameExists(const std::string& name);
    
    /**
     * 更新用户密码
     * @param userId 用户ID
     * @param newPassword 新密码
     * @return 错误码
     */
    ErrorCode updatePassword(int userId, const std::string& newPassword);
    
    /**
     * 删除用户
     * @param userId 用户ID
     * @return 错误码
     */
    ErrorCode deleteUser(int userId);
    
private:
    std::unique_ptr<SecureDB> _db;
    
    /**
     * 初始化数据库连接
     * @return 是否成功
     */
    bool initDatabase();
    
    /**
     * 从结果集构造User对象
     * @param row 数据库行
     * @return User对象
     */
    User constructUserFromRow(MYSQL_ROW row);
};

#endif // SECUREUSERMODEL_HPP