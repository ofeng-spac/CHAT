#include "../../../include/server/model/SecureUserModel.hpp"
#include "../../../include/server/security/PasswordUtils.hpp"
#include "../../../include/server/common/InputValidator.hpp"
#include "../../../include/server/common/Logger.hpp"
#include <muduo/base/Logging.h>
#include <vector>

SecureUserModel::SecureUserModel() {
    _db = std::make_unique<SecureDB>();
    if (!initDatabase()) {
        LOG_ERROR << "Failed to initialize database connection";
    }
}

SecureUserModel::~SecureUserModel() {
    // unique_ptr会自动清理
}

bool SecureUserModel::initDatabase() {
    return _db->connect();
}

ErrorCode SecureUserModel::insert(User& user) {
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
    
    // 检查用户名是否已存在
    if (isUsernameExists(user.getName())) {
        CHAT_LOG_ERROR_F("Username already exists: %s", user.getName().c_str());
        return ErrorCode::USER_ALREADY_EXISTS;
    }
    
    // 密码哈希处理
    string salt = PasswordUtils::generateSalt();
    string hashedPassword = PasswordUtils::hashPassword(user.getPwd(), salt);
    
    // 使用预编译语句插入用户
    string sql = "INSERT INTO user(name, password, salt, state) VALUES(?, ?, ?, ?)";
    vector<string> params = {
        user.getName(),
        hashedPassword,
        salt,
        user.getState()
    };
    
    if (_db->executeUpdate(sql, params)) {
        // 获取插入的用户ID
        unsigned long long insertId = _db->getLastInsertId();
        user.setId(static_cast<int>(insertId));
        CHAT_LOG_INFO_F("User inserted successfully with ID: %d", user.getId());
        return ErrorCode::SUCCESS;
    } else {
        CHAT_LOG_ERROR("Failed to insert user using prepared statement");
        return ErrorCode::DATABASE_INSERT_FAILED;
    }
}

std::pair<User, ErrorCode> SecureUserModel::queryById(int id) {
    CHAT_LOG_DEBUG_F("Querying user with ID: %d", id);
    
    // 输入验证
    ErrorCode validationResult = InputValidator::validateUserId(id);
    if (validationResult != ErrorCode::SUCCESS) {
        CHAT_LOG_ERROR_F("Invalid user ID: %d", id);
        return std::make_pair(User(), validationResult);
    }
    
    // 使用预编译语句查询
    string sql = "SELECT id, name, password, salt, state FROM user WHERE id = ?";
    vector<string> params = {std::to_string(id)};
    
    MYSQL_RES* res = _db->executeQuery(sql, params);
    if (res == nullptr) {
        CHAT_LOG_ERROR_F("Failed to query user with ID: %d", id);
        return std::make_pair(User(), ErrorCode::DATABASE_QUERY_FAILED);
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr) {
        mysql_free_result(res);
        CHAT_LOG_DEBUG_F("User not found with ID: %d", id);
        return std::make_pair(User(), ErrorCode::USER_NOT_FOUND);
    }
    
    User user = constructUserFromRow(row);
    mysql_free_result(res);
    
    CHAT_LOG_DEBUG_F("User found: %s", user.getName().c_str());
    return std::make_pair(user, ErrorCode::SUCCESS);
}

std::pair<User, ErrorCode> SecureUserModel::queryByName(const std::string& name) {
    CHAT_LOG_DEBUG_F("Querying user with name: %s", name.c_str());
    
    // 输入验证
    ErrorCode validationResult = InputValidator::validateUsername(name);
    if (validationResult != ErrorCode::SUCCESS) {
        CHAT_LOG_ERROR_F("Invalid username: %s", name.c_str());
        return std::make_pair(User(), validationResult);
    }
    
    // 使用预编译语句查询
    string sql = "SELECT id, name, password, salt, state FROM user WHERE name = ?";
    vector<string> params = {name};
    
    MYSQL_RES* res = _db->executeQuery(sql, params);
    if (res == nullptr) {
        CHAT_LOG_ERROR_F("Failed to query user with name: %s", name.c_str());
        return std::make_pair(User(), ErrorCode::DATABASE_QUERY_FAILED);
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr) {
        mysql_free_result(res);
        CHAT_LOG_DEBUG_F("User not found with name: %s", name.c_str());
        return std::make_pair(User(), ErrorCode::USER_NOT_FOUND);
    }
    
    User user = constructUserFromRow(row);
    mysql_free_result(res);
    
    CHAT_LOG_DEBUG_F("User found: %s", user.getName().c_str());
    return std::make_pair(user, ErrorCode::SUCCESS);
}

std::pair<User, ErrorCode> SecureUserModel::login(const std::string& name, const std::string& password) {
    CHAT_LOG_INFO_F("Login attempt for user: %s", name.c_str());
    
    // 输入验证
    ErrorCode validationResult = InputValidator::validateUsername(name);
    if (validationResult != ErrorCode::SUCCESS) {
        CHAT_LOG_ERROR_F("Invalid username for login: %s", name.c_str());
        return std::make_pair(User(), validationResult);
    }
    
    validationResult = InputValidator::validatePassword(password);
    if (validationResult != ErrorCode::SUCCESS) {
        CHAT_LOG_ERROR("Invalid password for login");
        return std::make_pair(User(), validationResult);
    }
    
    // 查询用户信息
    auto [user, queryResult] = queryByName(name);
    if (queryResult != ErrorCode::SUCCESS) {
        return std::make_pair(User(), queryResult);
    }
    
    // 验证密码
    string storedHash = user.getPwd(); // 这里存储的是哈希值
    string salt = user.getSalt();
    string inputHash = PasswordUtils::hashPassword(password, salt);
    
    if (storedHash != inputHash) {
        CHAT_LOG_ERROR_F("Password verification failed for user: %s", name.c_str());
        return std::make_pair(User(), ErrorCode::INVALID_PASSWORD);
    }
    
    CHAT_LOG_INFO_F("Login successful for user: %s", name.c_str());
    return std::make_pair(user, ErrorCode::SUCCESS);
}

ErrorCode SecureUserModel::updateState(int userId, const std::string& state) {
    CHAT_LOG_DEBUG_F("Updating state for user ID %d to: %s", userId, state.c_str());
    
    // 输入验证
    ErrorCode validationResult = InputValidator::validateUserId(userId);
    if (validationResult != ErrorCode::SUCCESS) {
        return validationResult;
    }
    
    // 验证状态值
    if (state != "online" && state != "offline") {
        CHAT_LOG_ERROR_F("Invalid state value: %s", state.c_str());
        return ErrorCode::INVALID_INPUT;
    }
    
    // 使用预编译语句更新状态
    string sql = "UPDATE user SET state = ? WHERE id = ?";
    vector<string> params = {state, std::to_string(userId)};
    
    if (_db->executeUpdate(sql, params)) {
        CHAT_LOG_DEBUG_F("State updated successfully for user ID: %d", userId);
        return ErrorCode::SUCCESS;
    } else {
        CHAT_LOG_ERROR_F("Failed to update state for user ID: %d", userId);
        return ErrorCode::DATABASE_UPDATE_FAILED;
    }
}

ErrorCode SecureUserModel::resetAllStates() {
    CHAT_LOG_INFO("Resetting all user states to offline");
    
    string sql = "UPDATE user SET state = 'offline'";
    vector<string> params; // 无参数
    
    if (_db->executeUpdate(sql, params)) {
        unsigned long long affectedRows = _db->getAffectedRows();
        CHAT_LOG_INFO_F("Reset %llu user states to offline", affectedRows);
        return ErrorCode::SUCCESS;
    } else {
        CHAT_LOG_ERROR("Failed to reset user states");
        return ErrorCode::DATABASE_UPDATE_FAILED;
    }
}

bool SecureUserModel::isUsernameExists(const std::string& name) {
    string sql = "SELECT COUNT(*) FROM user WHERE name = ?";
    vector<string> params = {name};
    
    MYSQL_RES* res = _db->executeQuery(sql, params);
    if (res == nullptr) {
        return false; // 查询失败，假设不存在
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    bool exists = false;
    if (row != nullptr && row[0] != nullptr) {
        int count = std::stoi(row[0]);
        exists = (count > 0);
    }
    
    mysql_free_result(res);
    return exists;
}

ErrorCode SecureUserModel::updatePassword(int userId, const std::string& newPassword) {
    CHAT_LOG_INFO_F("Updating password for user ID: %d", userId);
    
    // 输入验证
    ErrorCode validationResult = InputValidator::validateUserId(userId);
    if (validationResult != ErrorCode::SUCCESS) {
        return validationResult;
    }
    
    validationResult = InputValidator::validatePassword(newPassword);
    if (validationResult != ErrorCode::SUCCESS) {
        return validationResult;
    }
    
    // 生成新的盐值和哈希
    string salt = PasswordUtils::generateSalt();
    string hashedPassword = PasswordUtils::hashPassword(newPassword, salt);
    
    // 使用预编译语句更新密码
    string sql = "UPDATE user SET password = ?, salt = ? WHERE id = ?";
    vector<string> params = {hashedPassword, salt, std::to_string(userId)};
    
    if (_db->executeUpdate(sql, params)) {
        CHAT_LOG_INFO_F("Password updated successfully for user ID: %d", userId);
        return ErrorCode::SUCCESS;
    } else {
        CHAT_LOG_ERROR_F("Failed to update password for user ID: %d", userId);
        return ErrorCode::DATABASE_UPDATE_FAILED;
    }
}

ErrorCode SecureUserModel::deleteUser(int userId) {
    CHAT_LOG_INFO_F("Deleting user with ID: %d", userId);
    
    // 输入验证
    ErrorCode validationResult = InputValidator::validateUserId(userId);
    if (validationResult != ErrorCode::SUCCESS) {
        return validationResult;
    }
    
    // 使用预编译语句删除用户
    string sql = "DELETE FROM user WHERE id = ?";
    vector<string> params = {std::to_string(userId)};
    
    if (_db->executeUpdate(sql, params)) {
        unsigned long long affectedRows = _db->getAffectedRows();
        if (affectedRows > 0) {
            CHAT_LOG_INFO_F("User deleted successfully with ID: %d", userId);
            return ErrorCode::SUCCESS;
        } else {
            CHAT_LOG_ERROR_F("User not found with ID: %d", userId);
            return ErrorCode::USER_NOT_FOUND;
        }
    } else {
        CHAT_LOG_ERROR_F("Failed to delete user with ID: %d", userId);
        return ErrorCode::DATABASE_DELETE_FAILED;
    }
}

User SecureUserModel::constructUserFromRow(MYSQL_ROW row) {
    User user;
    if (row[0]) user.setId(std::stoi(row[0]));
    if (row[1]) user.setName(row[1]);
    if (row[2]) user.setPwd(row[2]); // 存储哈希值
    if (row[3]) user.setSalt(row[3]);
    if (row[4]) user.setState(row[4]);
    return user;
}