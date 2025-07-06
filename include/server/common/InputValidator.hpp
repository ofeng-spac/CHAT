#ifndef INPUTVALIDATOR_HPP
#define INPUTVALIDATOR_HPP

#include <string>
#include <regex>
#include "ErrorCodes.hpp"
#include "../../../thirdparty/json.hpp"

using json = nlohmann::json;
using namespace std;

/**
 * 输入验证工具类
 * 提供各种输入数据的验证功能，防止SQL注入和其他安全问题
 */
class InputValidator {
public:
    // 用户名验证配置
    static const int MIN_USERNAME_LENGTH = 3;
    static const int MAX_USERNAME_LENGTH = 20;
    
    // 密码验证配置
    static const int MIN_PASSWORD_LENGTH = 6;
    static const int MAX_PASSWORD_LENGTH = 50;
    
    // 消息验证配置
    static const int MAX_MESSAGE_LENGTH = 1000;
    
    // 群组名验证配置
    static const int MIN_GROUPNAME_LENGTH = 2;
    static const int MAX_GROUPNAME_LENGTH = 30;
    
    /**
     * 验证用户名格式
     * @param username 用户名
     * @return 验证结果错误码
     */
    static ErrorCode validateUsername(const string& username);
    
    /**
     * 验证密码格式
     * @param password 密码
     * @return 验证结果错误码
     */
    static ErrorCode validatePassword(const string& password);
    
    /**
     * 验证消息内容
     * @param message 消息内容
     * @return 验证结果错误码
     */
    static ErrorCode validateMessage(const string& message);
    
    /**
     * 验证群组名格式
     * @param groupName 群组名
     * @return 验证结果错误码
     */
    static ErrorCode validateGroupName(const string& groupName);
    
    /**
     * 验证用户ID
     * @param userId 用户ID
     * @return 验证结果错误码
     */
    static ErrorCode validateUserId(int userId);
    
    /**
     * 验证群组ID
     * @param groupId 群组ID
     * @return 验证结果错误码
     */
    static ErrorCode validateGroupId(int groupId);
    
    /**
     * 验证JSON格式
     * @param jsonStr JSON字符串
     * @return 验证结果错误码
     */
    static ErrorCode validateJsonFormat(const string& jsonStr);
    
    /**
     * 检查JSON中是否包含必需字段
     * @param jsonObj JSON对象
     * @param requiredFields 必需字段列表
     * @return 验证结果错误码
     */
    static ErrorCode validateRequiredFields(const json& jsonObj, const vector<string>& requiredFields);
    
    /**
     * 清理和转义字符串，防止SQL注入
     * @param input 输入字符串
     * @return 清理后的字符串
     */
    static string sanitizeString(const string& input);
    
    /**
     * 检查字符串是否包含危险字符
     * @param input 输入字符串
     * @return 是否包含危险字符
     */
    static bool containsDangerousChars(const string& input);
    
    /**
     * 验证登录请求
     * @param jsonObj 登录请求JSON
     * @return 验证结果错误码
     */
    static ErrorCode validateLoginRequest(const json& jsonObj);
    
    /**
     * 验证注册请求
     * @param jsonObj 注册请求JSON
     * @return 验证结果错误码
     */
    static ErrorCode validateRegisterRequest(const json& jsonObj);
    
    /**
     * 验证聊天消息请求
     * @param jsonObj 聊天消息请求JSON
     * @return 验证结果错误码
     */
    static ErrorCode validateChatMessageRequest(const json& jsonObj);
    
    /**
     * 验证添加好友请求
     * @param jsonObj 添加好友请求JSON
     * @return 验证结果错误码
     */
    static ErrorCode validateAddFriendRequest(const json& jsonObj);
    
    /**
     * 验证创建群组请求
     * @param jsonObj 创建群组请求JSON
     * @return 验证结果错误码
     */
    static ErrorCode validateCreateGroupRequest(const json& jsonObj);
    
private:
    /**
     * 检查字符串长度是否在指定范围内
     * @param str 字符串
     * @param minLen 最小长度
     * @param maxLen 最大长度
     * @return 是否在范围内
     */
    static bool isLengthValid(const string& str, int minLen, int maxLen);
    
    /**
     * 检查字符串是否只包含允许的字符
     * @param str 字符串
     * @param pattern 正则表达式模式
     * @return 是否匹配模式
     */
    static bool matchesPattern(const string& str, const string& pattern);
};

#endif // INPUTVALIDATOR_HPP