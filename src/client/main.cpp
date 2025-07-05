#include "json.hpp"
#include<iostream>
#include<thread>
#include<string>
#include<vector>
#include<chrono>
#include<ctime>
#include<unordered_map>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"group.hpp"
#include"user.hpp"
#include"public.hpp"

// 全局变量
bool isMainMenuRunning = true;  // 控制主菜单程序运行
int g_clientfd = -1;  // 修改为全局变量，并重命名

//记录当前系统登陆的用户信息
User g_currentUser;
//记录当前登陆用户的好友列表信息
vector<User> g_currentUserFriendList;
//记录当前用户所在的群组列表信息
vector<Group> g_currentUserGroupList;

// 函数前向声明
void showCurrentUserDate();
void readTaskHandler(int clientfd);
string getCurrentTime();
void mainMenu();
void help(int clientfd, string str);
void chat(int clientfd, string str);
void addfriend(int clientfd, string str);
void creategroup(int clientfd, string str);
void addgroup(int clientfd, string str);
void groupchat(int clientfd, string str);
void loginout(int clientfd, string str);

//系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}
};
//注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)> > commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}
};


int main(int argc,char *argv[])
{
    if(argc<3)
    {
        cerr<<"command invalid,please input as: "<<endl;
        cerr<<"./ChatClient 127.0.0.1 6000"<<endl;
        exit(-1);
    }

    //解析通过命令行参数传递的ip地址和端口号
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    //创建客户端的socket
    g_clientfd = socket(AF_INET,SOCK_STREAM,0);  // 使用全局变量
    if(g_clientfd == -1)
    {
        cerr<<"socket create error"<<endl;
        exit(-1);
    }
    //填写服务器的地址信息
    sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(ip);
    //与服务器建立连接
    if(connect(g_clientfd,(sockaddr*)&serveraddr,sizeof(serveraddr)) == -1)  // 使用全局变量
    {
        cerr<<"connect error"<<endl;
        close(g_clientfd);  // 使用全局变量
        exit(-1);
    }
    //main线程用于接收用户输入，负责发送数据
    while(isMainMenuRunning)
    {
        //显示当前用户的基本信息
        cout<<"=================="<<endl;
        cout<<"1:login 2:register 3:exit"<<endl;
        cout<<"=================="<<endl;
        cout<<"please input your select:";
        int select = 0;
        cin>>select;
        cin.get();//清除缓冲区
        switch(select)
        {
            case 1://login业务
            {
                int id = 0;
                char pwd[50] = {0};
                cout<<"please input your id:";
                string id_str;
                cin >> id_str;
                // Validate if input is a number
                try {
                    id = stoi(id_str);
                } catch (const std::exception& e) {
                    cerr << "Invalid ID format. Please enter a number." << endl;
                    break;
                }
                cin.get(); // Clear buffer
                cout<<"please input your pwd:";
                cin.getline(pwd,50);
                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["pwd"] = pwd;
                string request = js.dump();
                int len = send(g_clientfd,request.c_str(),request.size() + 1,0);
                if(len == -1)
                {
                    cerr<<"send login msg error"<<request<<endl;
                }
                else
                {
                    //接收服务器返回的登录结果
                    char buffer[1024] = {0};
                    int len = recv(g_clientfd,buffer,1024,0);
                    if(len == -1)
                    {
                        cerr<<"recv login msg error"<<endl;
                    }
                    else
                    {
                        // Check if buffer is empty or contains valid data
                        if (strlen(buffer) == 0) {
                            cerr << "Received empty response from server" << endl;
                            break;
                        }
                        
                        try {
                            json js = json::parse(buffer);
                            if(js["errno"].get<int>()!= 0)
                            {
                                cerr<<"login failed: "<<js["errmsg"]<<endl;
                            }
                            else
                            {
                                //记录当前用户的信息
                                g_currentUser.setId(id);
                                g_currentUser.setName(js["name"]);
                                
                                //记录当前用户的好友列表信息
                                if(js.contains("friends"))
                                {
                                    //初始化
                                    g_currentUserFriendList.clear();
                                    vector<string> vec = js["friends"];
                                    for(string &str:vec)
                                    {
                                        json js = json::parse(str);
                                        User user(js["id"],js["name"],js["state"]);
                                        g_currentUserFriendList.push_back(user);
                                    }
                                }
                                //记录当前用户的群组列表信息
                                if(js.contains("groups"))
                                {
                                    vector<string> vec1 = js["groups"];
                                    for(string &groupstr:vec1)
                                    {
                                        json groupjs = json::parse(groupstr);
                                        Group group(groupjs["id"],groupjs["groupname"],groupjs["groupdesc"]);
                                        
                                        vector<string> vec2 = groupjs["users"];
                                        for(string &userstr:vec2)
                                        {
                                            json userjs = json::parse(userstr);
                                            GroupUser groupUser;
                                            groupUser.setId(userjs["id"].get<int>());
                                            groupUser.setName(userjs["name"]);
                                            groupUser.setRole(userjs["role"]);
                                            group.getUsers().push_back(groupUser);
                                        }
                                        g_currentUserGroupList.push_back(group);
                                    }
                                }
                                //显示当前用户的基本信息
                                showCurrentUserDate();

                                //显示当前用户的离线消息，个人聊天信息或者群组信息
                                if(js.contains("offlinemsg"))
                                {
                                    vector<string> vec = js["offlinemsg"];
                                    for(string &str:vec)
                                    {
                                        json js = json::parse(str);
                                        int msgtype = js["msgid"].get<int>();
                                        if(ONE_CHAT_MSG == msgtype)
                                        {
                                            // 一对一聊天消息
                                            cout << js["time"].get<string>() << "[" << js["id"].get<int>() << "]" << js["name"] << " say: " << js["msg"].get<string>() << endl;
                                        }
                                        else if(GROUP_CHAT_MSG == msgtype)
                                        {
                                            // 群聊消息
                                            cout << "群消息["<<js["groupid"]<<"]:"<<js["time"].get<string>() << "[" << js["id"].get<int>() << "]" << js["name"] << " say: " << js["msg"].get<string>() << endl;
                                        }
                                    
                                    }
                                }
                                static int threadnumber = 0;
                                if(threadnumber == 0)
                                {
                                    //登陆成功，启动接受线程
                                    thread readTask(readTaskHandler,g_clientfd);
                                    readTask.detach();
                                    threadnumber++;
                                }
                                

                                isMainMenuRunning = true;
                                mainMenu();//进入聊天主界面
                             }
                         } catch (const json::parse_error& e) {
                             cerr << "JSON parse error: " << e.what() << endl;
                             cerr << "Server response: " << buffer << endl;
                         }
                     }
                 }
                 break;
             }
             case 2://register业务
             {
                 char name[50] = {0};
                 char pwd[50] = {0};
                 cout<<"please input your name:";
                 cin.getline(name,50);
                 cout<<"please input your pwd:";
                 cin.getline(pwd,50);
                 json js;
                 js["msgid"] = REG_MSG;
                 js["name"] = name;
                 js["pwd"] = pwd;
                 string request = js.dump();
                 int len = send(g_clientfd,request.c_str(),request.size() + 1,0);
                 if(len == -1)
                 {
                     cerr<<"send register msg error"<<request<<endl;
                 }
                 else
                 {
                     char buffer[1024] = {0};
                     int len = recv(g_clientfd,buffer,1024,0);
                     if(len == -1)
                     {
                         cerr<<"recv register msg error"<<endl;
                     }
                     else
                     {
                         try {
                             json js = json::parse(buffer);
                             if(js["errno"].get<int>() != 0)
                             {
                                 cerr<<"register failed: "<<js["errmsg"]<<endl;
                             }
                             else
                             {
                                 cout<<"register success, userid is "<<js["id"]<<", do not forget it!"<<endl;
                             }
                         } catch (const json::parse_error& e) {
                             cerr << "JSON parse error in registration: " << e.what() << endl;
                             cerr << "Server response: " << buffer << endl;
                         }
                     }
                 }
                 break;
             }
             case 3://exit业务
             {
                 close(g_clientfd);
                 exit(0);
             }
             default:
                 cerr<<"invalid input!"<<endl;
                 break;
         }
     }
     return 0;
}


//显示当前登陆成功用户的基本信息
void showCurrentUserDate()
{
    cout<< "======================login user===================="<<endl;
    cout<< "current login user id: "<< g_currentUser.getId()<<"name: "<<g_currentUser.getName()<<endl;
    cout<< "friend list "<<endl;
    if(!g_currentUserFriendList.empty())
    {
        for(auto &user:g_currentUserFriendList)
        {
            cout<< "id: "<<user.getId()<<"name: "<<user.getName()<<"user state: "<<user.getState()<<endl;
        }
    }
    cout<<"-------------------group list---------------------"<<endl;
    if(!g_currentUserGroupList.empty())
    {
        for(auto &group:g_currentUserGroupList)
        {
            cout<< "group id: "<<group.getId()<<"group name: "<<group.getName()<<"group desc: "<<group.getDesc()<<endl;
            for(auto &user:group.getUsers())
            {
                cout<< "user id: "<<user.getId()<<"user name: "<<user.getName()<<"user state: "<<user.getState()<<endl;
            }
        }
    }
    cout<< "======================login user===================="<<endl;
}

string getCurrentTime()
{
    auto now = chrono::system_clock::now();
    time_t time = chrono::system_clock::to_time_t(now);
    string timeStr = ctime(&time);
    timeStr = timeStr.substr(0, timeStr.length()-1);
    return timeStr;
}

// 接收线程
void readTaskHandler(int clientfd)
{
    while(isMainMenuRunning)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if(len == -1 || len == 0)
        {
            cerr << "recv error or server closed" << endl;
            close(clientfd);
            exit(-1);
        }

        // 接收ChatServer转发的数据，反序列化生成json数据对象
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if(ONE_CHAT_MSG == msgtype)
        {
            // 一对一聊天消息
            cout << js["time"].get<string>() << "[" << js["id"].get<int>() << "]" << js["name"] << " say: " << js["msg"].get<string>() << endl;
            continue;
        }
        else if(GROUP_CHAT_MSG == msgtype)
        {
            // 群聊消息
            cout << "群消息["<<js["groupid"]<<"]:"<<js["time"].get<string>() << "[" << js["id"].get<int>() << "]" << js["name"] << " say: " << js["msg"].get<string>() << endl;
            continue;
            
        }

    }
}

// 主聊天界面
void mainMenu()
{
    help(g_clientfd, "");
    char buffer[1024] = {0};
    while(isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令
        int idx = commandbuf.find(":");
        if(-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            cout << "invalid command, please input help to see all commands" << endl;
            continue;
        }
        //调用对应的命令处理函数,mainMenu对修改封闭
        it->second(g_clientfd, commandbuf.substr(idx+1, commandbuf.size() - idx));//调用命令处理方法

    }
}

// 显示所有支持的命令
void help(int clientfd, string str)
{
    cout << "show command list >>> " << endl;
    cout << "help : 显示所有支持的命令" << endl;
    cout << "chat:friendid:message : 一对一聊天" << endl;
    cout << "addfriend:friendid : 添加好友" << endl;
    cout << "creategroup:groupname:groupdesc : 创建群组" << endl;
    cout << "addgroup:groupid : 加入群组" << endl;
    cout << "groupchat:groupid:message : 群聊" << endl;
    cout << "loginout : 注销" << endl;
    cout << "--------------------------------" << endl;
}
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string request = js.dump();
    int len = send(clientfd, request.c_str(), request.size() + 1, 0);
    if(len == -1)
    {
        cerr << "send add friend msg error" << request << endl;
    }
}
void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if(-1 == idx)
    {
        cout << "invalid command, please input help to see all commands" << endl;
        return;
    }
    int friendid = atoi(str.substr(0, idx).c_str());
    string msg = str.substr(idx+1, str.size() - idx);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string request = js.dump();
    int len = send(clientfd, request.c_str(), request.size() + 1, 0);
    if(len == -1)
    {
        cerr << "send chat msg error" << request << endl;
    }
}
// 创建群组
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if(-1 == idx)
    {
        cout << "invalid command, please input help to see all commands" << endl;
        return;
    }
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx+1, str.size() - idx);
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string request = js.dump();
    int len = send(clientfd, request.c_str(), request.size() + 1, 0);
    if(len == -1)
    {
        cerr << "send create group msg error" << request << endl;
    }
}

// 加入群组
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string request = js.dump();
    int len = send(clientfd, request.c_str(), request.size() + 1, 0);
    if(len == -1)
    {
        cerr << "send add group msg error" << request << endl;
    }
}

// 群聊
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if(-1 == idx)
    {
        cout << "invalid command, please input help to see all commands" << endl;
        return;
    }
    int groupid = atoi(str.substr(0, idx).c_str());
    string msg = str.substr(idx+1, str.size() - idx);
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string request = js.dump();
    int len = send(clientfd, request.c_str(), request.size() + 1, 0);
    if(len == -1)
    {
        cerr << "send group chat msg error" << request << endl;
    }
}

// 注销
void loginout(int clientfd, string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string request = js.dump();
    int len = send(clientfd, request.c_str(), request.size() + 1, 0);
    if(len == -1)
    {
        cerr << "send loginout msg error" << request << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}
