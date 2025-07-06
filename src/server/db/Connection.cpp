
#include "pch.h"
#include "Connection.h"

Connection::Connection()
{
    this->conn = mysql_init(nullptr);
    if (this->conn == nullptr) {
        cout << "mysql_init failed!" << endl;
    }
    cout << "Connection()" << endl;
}

Connection::~Connection()
{
    lock_guard<mutex> lock(conn_mutex);
    if(this->conn != nullptr)
    {
        // 清理任何未处理的结果
        MYSQL_RES* result;
        while ((result = mysql_use_result(conn)) != nullptr) {
            mysql_free_result(result);
        }
        mysql_close(this->conn);
        this->conn = nullptr;
    }
    cout << "~Connection()" << endl;
}

bool Connection::connect(string ip,unsigned short port,string user,string password,string dbname)
{
    // 设置连接选项
    my_bool reconnect = 1;
    mysql_options(this->conn, MYSQL_OPT_RECONNECT, &reconnect);
    
    // 设置连接超时
    unsigned int timeout = 10;
    mysql_options(this->conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    mysql_options(this->conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(this->conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
    
    // 设置字符集
    mysql_options(this->conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    
    MYSQL *p = mysql_real_connect(this->conn,ip.c_str(),user.c_str(),password.c_str(),dbname.c_str(),port,nullptr,0);
    if (p == nullptr) {
        cout << "MySQL connection failed: " << mysql_error(this->conn) << endl;
        cout << "Connection params: ip=" << ip << ", port=" << port << ", user=" << user << ", dbname=" << dbname << endl;
        return false;
    }
    
    // 连接成功后设置会话级别的选项
    mysql_query(this->conn, "SET SESSION sql_mode='STRICT_TRANS_TABLES,NO_ZERO_DATE,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO'");
    
    return true;
}

bool Connection::update(string sql)
{
    std::lock_guard<std::mutex> lock(conn_mutex);
    
    // 检查连接是否有效
    if (!isValid()) {
        cout << "Connection is invalid" << endl;
        return false;
    }
    
    // 清理之前可能未处理完的结果
    while (mysql_next_result(conn) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) mysql_free_result(result);
    }
    
    if(mysql_query(this->conn,sql.c_str())!=0)
    {
        cout<<"update error:"<<mysql_error(this->conn)<<endl;
        return false;
    }
    return true;
}

MYSQL_RES* Connection::query(string sql)
{
    std::lock_guard<std::mutex> lock(conn_mutex);
    
    // 检查连接是否有效
    if (!isValid()) {
        cout << "Connection is invalid" << endl;
        return nullptr;
    }
    
    // 清理之前可能未处理完的结果
    while (mysql_next_result(conn) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) mysql_free_result(result);
    }
    
    if(mysql_query(this->conn,sql.c_str())!=0)
    {
        cout<<"query error:"<<mysql_error(this->conn)<<endl;
        return nullptr;
    }
    // 使用mysql_store_result而不是mysql_use_result来避免"Commands out of sync"错误
    // mysql_store_result会立即获取所有结果，而mysql_use_result需要逐行读取
    return mysql_store_result(this->conn);
}

bool Connection::isValid() {
    lock_guard<mutex> lock(conn_mutex);
    if (conn == nullptr) {
        return false;
    }
    
    // 使用mysql_ping检查连接是否有效
    int ping_result = mysql_ping(conn);
    if (ping_result != 0) {
        cout << "Connection ping failed: " << mysql_error(conn) << endl;
        return false;
    }
    return true;
}

MYSQL* Connection::getConnection() {
    return conn;
}
