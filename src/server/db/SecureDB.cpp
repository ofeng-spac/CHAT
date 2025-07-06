#include "../../../include/server/db/SecureDB.hpp"
#include "../../../include/public.hpp"
#include <muduo/base/Logging.h>
#include <cstring>

// 数据库配置信息（应该从配置文件读取）
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

SecureDB::SecureDB() : _conn(nullptr) {
    _conn = mysql_init(nullptr);
    if (_conn == nullptr) {
        LOG_ERROR << "Failed to initialize MySQL connection";
    }
}

SecureDB::~SecureDB() {
    close();
}

bool SecureDB::connect() {
    if (_conn == nullptr) {
        LOG_ERROR << "MySQL connection not initialized";
        return false;
    }
    
    MYSQL* result = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                      password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    
    if (result == nullptr) {
        logMySQLError("connect");
        return false;
    }
    
    // 设置字符集
    if (mysql_set_character_set(_conn, "utf8mb4") != 0) {
        logMySQLError("set character set");
        return false;
    }
    
    // 设置自动提交模式
    if (mysql_autocommit(_conn, 1) != 0) {
        logMySQLError("set autocommit");
        return false;
    }
    
    LOG_INFO << "SecureDB connected successfully";
    return true;
}

void SecureDB::close() {
    if (_conn != nullptr) {
        mysql_close(_conn);
        _conn = nullptr;
    }
}

bool SecureDB::executeUpdate(const string& sql, const vector<string>& params) {
    if (_conn == nullptr) {
        LOG_ERROR << "Database not connected";
        return false;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(_conn);
    if (stmt == nullptr) {
        logMySQLError("stmt_init");
        return false;
    }
    
    // 预编译SQL语句
    if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
        LOG_ERROR << "Failed to prepare statement: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return false;
    }
    
    // 检查参数数量
    unsigned long param_count = mysql_stmt_param_count(stmt);
    if (param_count != params.size()) {
        LOG_ERROR << "Parameter count mismatch. Expected: " << param_count 
                  << ", Got: " << params.size();
        mysql_stmt_close(stmt);
        return false;
    }
    
    // 绑定参数
    if (param_count > 0 && !bindParameters(stmt, params)) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    // 执行语句
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "Failed to execute statement: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return false;
    }
    
    mysql_stmt_close(stmt);
    return true;
}

MYSQL_RES* SecureDB::executeQuery(const string& sql, const vector<string>& params) {
    if (_conn == nullptr) {
        LOG_ERROR << "Database not connected";
        return nullptr;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(_conn);
    if (stmt == nullptr) {
        logMySQLError("stmt_init");
        return nullptr;
    }
    
    // 预编译SQL语句
    if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
        LOG_ERROR << "Failed to prepare statement: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    // 检查参数数量
    unsigned long param_count = mysql_stmt_param_count(stmt);
    if (param_count != params.size()) {
        LOG_ERROR << "Parameter count mismatch. Expected: " << param_count 
                  << ", Got: " << params.size();
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    // 绑定参数
    if (param_count > 0 && !bindParameters(stmt, params)) {
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    // 执行语句
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "Failed to execute statement: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    // 获取结果
    MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
    if (result == nullptr) {
        LOG_ERROR << "No result metadata: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    // 注意：这里需要特殊处理，因为预编译语句的结果处理与普通查询不同
    // 为了兼容现有代码，我们使用mysql_stmt_store_result
    if (mysql_stmt_store_result(stmt) != 0) {
        LOG_ERROR << "Failed to store result: " << mysql_stmt_error(stmt);
        mysql_free_result(result);
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    // 注意：这里返回的result需要特殊处理
    // 实际项目中建议重新设计接口，使用更安全的结果处理方式
    mysql_stmt_close(stmt);
    return result;
}

bool SecureDB::bindParameters(MYSQL_STMT* stmt, const vector<string>& params) {
    if (params.empty()) {
        return true;
    }
    
    vector<MYSQL_BIND> bind_params(params.size());
    vector<unsigned long> lengths(params.size());
    
    // 初始化绑定结构
    memset(bind_params.data(), 0, sizeof(MYSQL_BIND) * params.size());
    
    for (size_t i = 0; i < params.size(); ++i) {
        bind_params[i].buffer_type = MYSQL_TYPE_STRING;
        bind_params[i].buffer = const_cast<char*>(params[i].c_str());
        bind_params[i].buffer_length = params[i].length();
        bind_params[i].length = &lengths[i];
        lengths[i] = params[i].length();
        bind_params[i].is_null = 0;
    }
    
    if (mysql_stmt_bind_param(stmt, bind_params.data()) != 0) {
        LOG_ERROR << "Failed to bind parameters: " << mysql_stmt_error(stmt);
        return false;
    }
    
    return true;
}

unsigned long long SecureDB::getLastInsertId() {
    if (_conn == nullptr) {
        return 0;
    }
    return mysql_insert_id(_conn);
}

unsigned long long SecureDB::getAffectedRows() {
    if (_conn == nullptr) {
        return 0;
    }
    return mysql_affected_rows(_conn);
}

bool SecureDB::beginTransaction() {
    if (_conn == nullptr) {
        return false;
    }
    
    if (mysql_autocommit(_conn, 0) != 0) {
        logMySQLError("begin transaction");
        return false;
    }
    
    return true;
}

bool SecureDB::commit() {
    if (_conn == nullptr) {
        return false;
    }
    
    if (mysql_commit(_conn) != 0) {
        logMySQLError("commit");
        return false;
    }
    
    // 恢复自动提交
    mysql_autocommit(_conn, 1);
    return true;
}

bool SecureDB::rollback() {
    if (_conn == nullptr) {
        return false;
    }
    
    if (mysql_rollback(_conn) != 0) {
        logMySQLError("rollback");
        return false;
    }
    
    // 恢复自动提交
    mysql_autocommit(_conn, 1);
    return true;
}

string SecureDB::escapeString(const string& input) {
    if (_conn == nullptr || input.empty()) {
        return input;
    }
    
    // 分配足够的缓冲区（最坏情况下每个字符都需要转义）
    vector<char> buffer(input.length() * 2 + 1);
    
    unsigned long escaped_length = mysql_real_escape_string(_conn, 
                                                           buffer.data(), 
                                                           input.c_str(), 
                                                           input.length());
    
    return string(buffer.data(), escaped_length);
}

void SecureDB::logMySQLError(const string& operation) {
    if (_conn != nullptr) {
        LOG_ERROR << "MySQL " << operation << " failed: " 
                  << mysql_error(_conn) << " (Error: " << mysql_errno(_conn) << ")";
    } else {
        LOG_ERROR << "MySQL " << operation << " failed: Connection is null";
    }
}