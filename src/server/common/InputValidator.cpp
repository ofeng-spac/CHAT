#include "server/common/InputValidator.hpp"
#include <muduo/base/Logging.h>
#include <algorithm>
#include <cctype>

ErrorCode InputValidator::validateUsername(const string& username) {
    // 检查长度
    if (!isLengthValid(username, MIN_USERNAME_LENGTH, MAX_USERNAME_LENGTH)) {
        if (username.length() < MIN_USERNAME_LENGTH) {
            return ErrorCode::USERNAME_TOO_SHORT;
        } else {
            return ErrorCode::USERNAME_TOO_LONG;
        }
    }
    
    // 检查字符格式：只允许字母、数字和下划线
    if (!matchesPattern(username, "^[a-zA-Z0-9_]+$")) {
        return ErrorCode::INVALID_USERNAME;
    }
    
    // 检查是否包含危险字符
    if (containsDangerousChars(username)) {
        return ErrorCode::INVALID_USERNAME;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validatePassword(const string& password) {
    // 检查长度
    if (!isLengthValid(password, MIN_PASSWORD_LENGTH, MAX_PASSWORD_LENGTH)) {
        if (password.length() < MIN_PASSWORD_LENGTH) {
            return ErrorCode::PASSWORD_TOO_SHORT;
        } else {
            return ErrorCode::PASSWORD_TOO_LONG;
        }
    }
    
    // 密码可以包含更多字符，但不能包含危险字符
    if (containsDangerousChars(password)) {
        return ErrorCode::INVALID_INPUT;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validateMessage(const string& message) {
    // 检查是否为空
    if (message.empty()) {
        return ErrorCode::MESSAGE_EMPTY;
    }
    
    // 检查长度
    if (message.length() > MAX_MESSAGE_LENGTH) {
        return ErrorCode::MESSAGE_TOO_LONG;
    }
    
    // 检查是否包含危险字符
    if (containsDangerousChars(message)) {
        return ErrorCode::INVALID_INPUT;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validateGroupName(const string& groupName) {
    // 检查长度
    if (!isLengthValid(groupName, MIN_GROUPNAME_LENGTH, MAX_GROUPNAME_LENGTH)) {
        return ErrorCode::INVALID_INPUT;
    }
    
    // 检查是否包含危险字符
    if (containsDangerousChars(groupName)) {
        return ErrorCode::INVALID_INPUT;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validateUserId(int userId) {
    if (userId <= 0) {
        return ErrorCode::PARAMETER_OUT_OF_RANGE;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validateGroupId(int groupId) {
    if (groupId <= 0) {
        return ErrorCode::PARAMETER_OUT_OF_RANGE;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validateJsonFormat(const string& jsonStr) {
    try {
        auto parsed = nlohmann::json::parse(jsonStr);
        (void)parsed; // 消除未使用变量警告
        return ErrorCode::SUCCESS;
    } catch (const nlohmann::json::exception& e) {
        LOG_ERROR << "Invalid JSON format: " << e.what();
        return ErrorCode::INVALID_JSON_FORMAT;
    }
}

ErrorCode InputValidator::validateRequiredFields(const json& jsonObj, const vector<string>& requiredFields) {
    for (const auto& field : requiredFields) {
        if (jsonObj.find(field) == jsonObj.end()) {
            LOG_ERROR << "Missing required field: " << field;
            return ErrorCode::MISSING_REQUIRED_FIELD;
        }
    }
    return ErrorCode::SUCCESS;
}

string InputValidator::sanitizeString(const string& input) {
    string result = input;
    
    // 替换危险字符
    replace(result.begin(), result.end(), '\0', ' ');
    
    // 移除或转义SQL注入相关字符
    string dangerous[] = {"'", "\"", ";", "--", "/*", "*/", "xp_", "sp_"};
    for (const auto& danger : dangerous) {
        size_t pos = 0;
        while ((pos = result.find(danger, pos)) != string::npos) {
            result.replace(pos, danger.length(), "");
            pos += 1;
        }
    }
    
    return result;
}

bool InputValidator::containsDangerousChars(const string& input) {
    // 检查SQL注入相关字符
    string dangerous[] = {"'", "\"", ";", "--", "/*", "*/", "xp_", "sp_", "<script", "</script>", "<", ">"};
    
    string lowerInput = input;
    transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
    
    for (const auto& danger : dangerous) {
        if (lowerInput.find(danger) != string::npos) {
            return true;
        }
    }
    
    // 检查控制字符
    for (char c : input) {
        if (iscntrl(c) && c != '\n' && c != '\r' && c != '\t') {
            return true;
        }
    }
    
    return false;
}

ErrorCode InputValidator::validateLoginRequest(const json& jsonObj) {
    // 检查必需字段
    vector<string> requiredFields = {"name", "password"};
    ErrorCode result = validateRequiredFields(jsonObj, requiredFields);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 验证用户名
    string username = jsonObj["name"];
    result = validateUsername(username);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 验证密码
    string password = jsonObj["password"];
    result = validatePassword(password);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validateRegisterRequest(const json& jsonObj) {
    // 检查必需字段
    vector<string> requiredFields = {"name", "password"};
    ErrorCode result = validateRequiredFields(jsonObj, requiredFields);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 验证用户名
    string username = jsonObj["name"];
    result = validateUsername(username);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 验证密码
    string password = jsonObj["password"];
    result = validatePassword(password);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validateChatMessageRequest(const json& jsonObj) {
    // 检查必需字段
    vector<string> requiredFields = {"id", "from", "to", "msg"};
    ErrorCode result = validateRequiredFields(jsonObj, requiredFields);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 验证用户ID
    int fromId = jsonObj["from"];
    result = validateUserId(fromId);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    int toId = jsonObj["to"];
    result = validateUserId(toId);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 验证消息内容
    string message = jsonObj["msg"];
    result = validateMessage(message);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validateAddFriendRequest(const json& jsonObj) {
    // 检查必需字段
    vector<string> requiredFields = {"id", "friendid"};
    ErrorCode result = validateRequiredFields(jsonObj, requiredFields);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 验证用户ID
    int userId = jsonObj["id"];
    result = validateUserId(userId);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    int friendId = jsonObj["friendid"];
    result = validateUserId(friendId);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 检查是否尝试添加自己为好友
    if (userId == friendId) {
        return ErrorCode::CANNOT_ADD_SELF;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode InputValidator::validateCreateGroupRequest(const json& jsonObj) {
    // 检查必需字段
    vector<string> requiredFields = {"groupname", "groupdesc"};
    ErrorCode result = validateRequiredFields(jsonObj, requiredFields);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 验证群组名
    string groupName = jsonObj["groupname"];
    result = validateGroupName(groupName);
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // 验证群组描述
    string groupDesc = jsonObj["groupdesc"];
    if (groupDesc.length() > 200) {
        return ErrorCode::INVALID_INPUT;
    }
    
    if (containsDangerousChars(groupDesc)) {
        return ErrorCode::INVALID_INPUT;
    }
    
    return ErrorCode::SUCCESS;
}

bool InputValidator::isLengthValid(const string& str, int minLen, int maxLen) {
    int len = str.length();
    return len >= minLen && len <= maxLen;
}

bool InputValidator::matchesPattern(const string& str, const string& pattern) {
    try {
        regex regexPattern(pattern);
        return regex_match(str, regexPattern);
    } catch (const regex_error& e) {
        LOG_ERROR << "Regex error: " << e.what();
        return false;
    }
}