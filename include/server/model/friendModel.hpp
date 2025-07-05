#ifndef FRIENDMODEL_HPP
#define FRIENDMODEL_HPP
#include<vector>
#include"user.hpp"

using namespace std;
class FriendModel
{
public:
    //添加好友关系
    void insert(int userid, int friendid);
    //返回用户好友列表
    vector<User> query(int userid);
    //删除好友关系
    void remove(int userid, int friendid);
private:
    //存储好友信息的表名
    string _friendTable = "friend";
};
#endif