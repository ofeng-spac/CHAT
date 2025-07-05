#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"


//群组用户
class GroupUser : public User
{
    public:
        void setRole(string role) { _role = role; }
        string getRole() { return _role; }
    private:
        string _role; //用户角色  创建人/普通成员
};
#endif