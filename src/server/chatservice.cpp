#include "chatservice.hpp"
#include "public.hpp"
#include "../../include/server/common/ErrorCodes.hpp"
#include<string>
#include<memory>
#include<vector>
#include<mutex>
#include<map>
#include "muduo/base/Logging.h"
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include"UserModel.hpp"


//获取单例对象的接口函数
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

//注册消息以及对应的处理函数
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});

    if(_redis.connect())
    {
        //设置上报消息的回调对象
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}


//处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["pwd"];
    auto result = _userModel.query(id);
    User user = result.first;
    ErrorCode error = result.second;
    if (error == ErrorCode::SUCCESS && user.getId() != -1 && user.getPwd() == pwd)
    {
        if(user.getState() == "online")
        {   //该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登陆，请重新输入新账号";
            conn->send(response.dump());
        }
        //登陆成功,记录用户连接信息
        {
            lock_guard<mutex> lock(_connMutex); //加锁保证线程安全
            _userConnMap.insert({id, conn});
        }
        //id用户登录成功后，向redis订阅channel(id)
        _redis.subscribe(id);

        user.setState("online");
        //更新用户状态信息
        _userModel.updateState(user);

        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        response["name"] = user.getName();
        //查询该用户是否有离线消息
        vector<string> vec = _offlineMsgModel.query(id);
        if (!vec.empty())
        {
            response["offlinemsg"] = vec;
            //读取该用户的离线消息后，把该用户的所有离线消息删除掉
            _offlineMsgModel.remove(id);
        }
        //查询该用户的好友信息并返回
        vector<User> userVec = _friendModel.query(id);
        if (!userVec.empty())
        {
            vector<string> friendVec;
            for (User &user : userVec)
            {
                json js;
                js["id"] = user.getId();
                js["name"] = user.getName();
                js["state"] = user.getState();
                friendVec.push_back(js.dump());  
            }
        }
        conn->send(response.dump());        
    }
    else
    {
        //该用户不存在，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
        

        conn->send(response.dump());
    }

    

}
//处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["pwd"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    ErrorCode state = _userModel.insert(user);
    if (state == ErrorCode::SUCCESS)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK; //注册响应
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "注册失败";
        conn->send(response.dump());
    }

}
//获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    //记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        //返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time){
            LOG_ERROR << "msgid: " << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}
//处理注册响应业务
void ChatService::regAck(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 处理注册响应的逻辑
    // 这里可以根据需要添加具体的处理逻辑
    // 例如，可以记录日志或者执行其他操作
    LOG_INFO << "Handling registration acknowledgment message";
}

//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                //从map表删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    //用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId());
    //更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}
//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            //toid在线，转发消息 服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;

        }
    }
    //查询toid是否在线
    auto result = _userModel.query(toid);
    User user = result.first;
    ErrorCode error = result.second;
    if (error == ErrorCode::SUCCESS && user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }
    //toid不在线，存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}
void ChatService::reset()
{
    //把online状态的用户，设置成offline
    _userModel.resetState();
}
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    //存储好友信息
    _friendModel.insert(userid, friendid);
}

//创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    //存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        //存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
    //返回创建成功的群组信息
}

//加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}
//群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid); 
    for (int id : useridVec)
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(id);
        if (it!= _userConnMap.end())
        {
            //toid在线，转发消息 服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
        else
        {
            //查询toid是否在线
            auto result = _userModel.query(id);
            if (result.second == ErrorCode::SUCCESS && result.first.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                //存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
            
        }
    }

}

void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it!= _userConnMap.end())
        {
            //从map表删除用户的连接信息
            _userConnMap.erase(it);

        }
    }
    //用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid);
    //更新用户的状态信息
    if (userid!= -1)
    {
        auto result = _userModel.query(userid);
        if (result.second == ErrorCode::SUCCESS) {
            User user = result.first;
            user.setState("offline");
            _userModel.updateState(user);
        }
    }
}
void ChatService::handleRedisSubscribeMessage(int userid, string msg)//从redis消息队列中获取订阅的消息
{
    json js = json::parse(msg);
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it!= _userConnMap.end())
    {
        it->second->send(js.dump());
        return;
    }
    _offlineMsgModel.insert(userid, js.dump());
}