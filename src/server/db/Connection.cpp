
#include "pch.h"
#include "Connection.h"

Connection::Connection()
{
    this->conn = mysql_init(nullptr);
    cout << "Connection()" << endl;
}

Connection::~Connection()
{
    if(this->conn != nullptr)
    {
        mysql_close(this->conn);
    }
    cout << "~Connection()" << endl;
}

bool Connection::connect(string ip,unsigned short port,string user,string password,string dbname)
{
    MYSQL *p = mysql_real_connect(this->conn,ip.c_str(),user.c_str(),password.c_str(),dbname.c_str(),port,nullptr,0);
    return p!=nullptr;
}

bool Connection::update(string sql)
{
    if(mysql_query(this->conn,sql.c_str())!=0)
    {
        cout<<"update error:"<<mysql_error(this->conn)<<endl;
        return false;
    }
    return true;
}

MYSQL_RES* Connection::query(string sql)
{
    if(mysql_query(this->conn,sql.c_str())!=0)
    {
        cout<<"query error:"<<mysql_error(this->conn)<<endl;
        return nullptr;
    }
    return mysql_use_result(this->conn);
}
