#ifndef USER_H
#define USER_H

#include<string>

using namespace std;
//匹配User表
class User{
public:
    User(int id=-1, string name="", string pwd="", string state="offline", string salt="")
    {
        this->id = id;
        this->name = name;
        this->pwd = pwd;
        this->state = state;
        this->salt = salt;
    }
    void setId(int id){
        this->id = id;
    }
    void setName(string name){
        this->name = name;
    }
    void setPwd(string pwd){
        this->pwd = pwd;
    }
    void setState(string state){
        this->state = state;
    }
    void setSalt(string salt){
        this->salt = salt;
    }
    int getId(){
        return this->id;
    }
    string getName(){
        return this->name;
    }
    string getPwd(){
        return this->pwd;
    }
    string getState(){
        return this->state;
    }
    string getSalt(){
        return this->salt;
    }

private:
    int id;
    string name;
    string pwd;
    string state;
    string salt;
};

#endif  // USER_H