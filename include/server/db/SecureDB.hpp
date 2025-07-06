#ifndef SECUREDB_HPP
#define SECUREDB_HPP

#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <memory>
#include "../common/ErrorCodes.hpp"

using namespace std;

/**
 * 安全数据库操作类
 * 使用预编译语句防止SQL注入攻击
 */
class SecureDB {
public:
    SecureDB();
    ~SecureDB();
    
    // 连接数据库
    bool connect();
    
    // 关闭连接
    void close();
    
    // 获取原始连接（仅在必要时使用）
    MYSQL* getConnection() { return _conn; }
    
    // 预编译语句相关操作
    
    /**
     * 执行预编译的插入/更新/删除操作
     * @param sql 带占位符的SQL语句
     * @param params 参数列表
     * @return 是否执行成功
     */
    bool executeUpdate(const string& sql, const vector<string>& params);
    
    /**
     * 执行预编译的查询操作
     * @param sql 带占位符的SQL语句
     * @param params 参数列表
     * @return 查询结果
     */
    MYSQL_RES* executeQuery(const string& sql, const vector<string>& params);
    
    /**
     * 获取最后插入的ID
     * @return 插入的ID
     */
    unsigned long long getLastInsertId();
    
    /**
     * 获取受影响的行数
     * @return 受影响的行数
     */
    unsigned long long getAffectedRows();
    
    /**
     * 开始事务
     * @return 是否成功
     */
    bool beginTransaction();
    
    /**
     * 提交事务
     * @return 是否成功
     */
    bool commit();
    
    /**
     * 回滚事务
     * @return 是否成功
     */
    bool rollback();
    
    /**
     * 转义字符串（备用方案）
     * @param input 输入字符串
     * @return 转义后的字符串
     */
    string escapeString(const string& input);
    
private:
    MYSQL* _conn;
    
    /**
     * 绑定参数到预编译语句
     * @param stmt 预编译语句
     * @param params 参数列表
     * @return 是否绑定成功
     */
    bool bindParameters(MYSQL_STMT* stmt, const vector<string>& params);
    
    /**
     * 记录MySQL错误
     */
    void logMySQLError(const string& operation);
};

#endif // SECUREDB_HPP