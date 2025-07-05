#ifndef PUBLIC_H
#define PUBLIC_H

/*
    server和client的公共文件
*/
enum EnMsgType
{
    LOGIN_MSG = 1, //登录消息
    REG_MSG , //注册消息
    REG_MSG_ACK, //注册响应消息
    LOGIN_MSG_ACK, //登录响应消息
    ONE_CHAT_MSG, //聊天消息
    ADD_FRIEND_MSG, //添加好友消息
    CREATE_GROUP_MSG, //创建群组
    ADD_GROUP_MSG, //加入群组
    GROUP_CHAT_MSG, //群聊天


    LOGINOUT_MSG, //注销消息
    LOGINOUT_MSG_ACK, //注销响应消息
    CREATE_GROUP_MSG_ACK, //创建群组响应消息
    ADD_GROUP_MSG_ACK, //加入群组响应消息
    ONE_CHAT_MSG_ACK, //聊天消息响应消息
    HEART_CHECK_MSG, // 心跳检测消息
    HEART_CHECK_MSG_ACK, // 心跳检测响应消息
    NAME_CHANGE_MSG, // 更改用户名消息
    NAME_CHANGE_MSG_ACK // 更改用户名响应消息
};

#endif  // PUBLIC_H
