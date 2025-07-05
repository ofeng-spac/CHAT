#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <string>
#include <thread>
#include <functional>
using namespace std;

class Redis{

public:
    Redis();
    ~Redis();
    bool connect();
    bool publish(int channel, string message);
    bool subscribe(int channel);
    bool unsubscribe(int channel);
    void observer_channel_message();//在独立线程中接收订阅通道的消息
    void init_notify_handler(function<void(int, string)> fn);//初始化向业务层上报消息的回调对象
private:
    redisContext* _publish_context; //hiredis的上下文对象，负责publish
    redisContext* _subscribe_context;//hiredis的上下文对象，负责subscribe
    function<void(int, string)> _notify_message_handler; //回调操作，收到订阅的消息，给service层上报

};
#endif