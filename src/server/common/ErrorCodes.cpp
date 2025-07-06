#include "server/common/ErrorCodes.hpp"
#include <muduo/base/Logging.h>

// 静态成员初始化
unordered_map<ErrorCode, string> ErrorHandler::errorMessages;

void ErrorHandler::initErrorMessages() {
    errorMessages = {
        // 成功
        {ErrorCode::SUCCESS, "操作成功"},
        
        // 用户相关错误
        {ErrorCode::USER_NOT_FOUND, "用户不存在"},
        {ErrorCode::USER_ALREADY_EXISTS, "用户已存在"},
        {ErrorCode::INVALID_PASSWORD, "密码错误"},
        {ErrorCode::USER_ALREADY_ONLINE, "用户已在线"},
        {ErrorCode::USER_NOT_ONLINE, "用户不在线"},
        {ErrorCode::INVALID_USERNAME, "用户名格式无效"},
        {ErrorCode::PASSWORD_TOO_SHORT, "密码长度不足"},
        {ErrorCode::PASSWORD_TOO_LONG, "密码长度过长"},
        {ErrorCode::USERNAME_TOO_SHORT, "用户名长度不足"},
        {ErrorCode::USERNAME_TOO_LONG, "用户名长度过长"},
        
        // 好友相关错误
        {ErrorCode::FRIEND_NOT_FOUND, "好友不存在"},
        {ErrorCode::FRIEND_ALREADY_EXISTS, "已经是好友关系"},
        {ErrorCode::CANNOT_ADD_SELF, "不能添加自己为好友"},
        {ErrorCode::FRIEND_REQUEST_PENDING, "好友请求待处理"},
        
        // 群组相关错误
        {ErrorCode::GROUP_NOT_FOUND, "群组不存在"},
        {ErrorCode::GROUP_ALREADY_EXISTS, "群组已存在"},
        {ErrorCode::NOT_GROUP_MEMBER, "不是群组成员"},
        {ErrorCode::ALREADY_GROUP_MEMBER, "已经是群组成员"},
        {ErrorCode::INSUFFICIENT_PERMISSION, "权限不足"},
        {ErrorCode::GROUP_FULL, "群组人数已满"},
        
        // 消息相关错误
        {ErrorCode::MESSAGE_TOO_LONG, "消息长度过长"},
        {ErrorCode::MESSAGE_EMPTY, "消息不能为空"},
        {ErrorCode::INVALID_MESSAGE_TYPE, "无效的消息类型"},
        {ErrorCode::MESSAGE_SEND_FAILED, "消息发送失败"},
        
        // 数据库相关错误
        {ErrorCode::DATABASE_CONNECTION_FAILED, "数据库连接失败"},
        {ErrorCode::DATABASE_QUERY_FAILED, "数据库查询失败"},
        {ErrorCode::DATABASE_UPDATE_FAILED, "数据库更新失败"},
        {ErrorCode::DATABASE_INSERT_FAILED, "数据库插入失败"},
        {ErrorCode::DATABASE_DELETE_FAILED, "数据库删除失败"},
        {ErrorCode::CONNECTION_POOL_EXHAUSTED, "连接池资源耗尽"},
        
        // 网络相关错误
        {ErrorCode::NETWORK_ERROR, "网络错误"},
        {ErrorCode::CONNECTION_LOST, "连接丢失"},
        {ErrorCode::INVALID_REQUEST_FORMAT, "请求格式无效"},
        {ErrorCode::REQUEST_TIMEOUT, "请求超时"},
        
        // Redis相关错误
        {ErrorCode::REDIS_CONNECTION_FAILED, "Redis连接失败"},
        {ErrorCode::REDIS_OPERATION_FAILED, "Redis操作失败"},
        
        // 系统相关错误
        {ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"},
        {ErrorCode::SERVICE_UNAVAILABLE, "服务不可用"},
        {ErrorCode::INVALID_CONFIGURATION, "配置无效"},
        {ErrorCode::RESOURCE_EXHAUSTED, "系统资源耗尽"},
        
        // 输入验证错误
        {ErrorCode::INVALID_INPUT, "输入无效"},
        {ErrorCode::MISSING_REQUIRED_FIELD, "缺少必需字段"},
        {ErrorCode::INVALID_JSON_FORMAT, "JSON格式无效"},
        {ErrorCode::PARAMETER_OUT_OF_RANGE, "参数超出范围"},
        {ErrorCode::INPUT_VALIDATION_EMPTY_FIELD, "字段不能为空"},
        {ErrorCode::INPUT_VALIDATION_SQL_INJECTION, "检测到SQL注入攻击"},
        {ErrorCode::INPUT_VALIDATION_XSS_ATTACK, "检测到XSS攻击"},
        {ErrorCode::INPUT_VALIDATION_COMMAND_INJECTION, "检测到命令注入攻击"},
        {ErrorCode::INPUT_VALIDATION_INVALID_FORMAT, "格式无效"},
        {ErrorCode::INPUT_VALIDATION_MALICIOUS_INPUT, "检测到恶意输入"},
        {ErrorCode::INPUT_VALIDATION_TOO_LONG, "输入长度超出限制"},
        {ErrorCode::INPUT_VALIDATION_INVALID_CHARACTERS, "包含无效字符"},
        {ErrorCode::INPUT_VALIDATION_DANGEROUS_FILE_TYPE, "危险的文件类型"},
        {ErrorCode::INPUT_VALIDATION_RESERVED_NAME, "使用了保留名称"},
        {ErrorCode::INPUT_VALIDATION_PATH_TRAVERSAL, "检测到路径遍历攻击"},
        {ErrorCode::INPUT_VALIDATION_ABSOLUTE_PATH, "不允许使用绝对路径"},
        {ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_SHORT, "密码长度不足8位"},
        {ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_LONG, "密码长度超出128位"},
        {ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_WEAK, "密码强度不足"},
        {ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_COMMON, "密码过于常见"},
        {ErrorCode::INPUT_VALIDATION_OUT_OF_RANGE, "数值超出允许范围"}
    };
}

void ErrorHandler::ensureInitialized() {
    static bool initialized = false;
    if (!initialized) {
        initErrorMessages();
        initialized = true;
    }
}

string ErrorHandler::getErrorMessage(ErrorCode code) {
    ensureInitialized();
    auto it = errorMessages.find(code);
    if (it != errorMessages.end()) {
        return it->second;
    }
    return "未知错误";
}

json ErrorHandler::createErrorResponse(ErrorCode code, const string& customMessage) {
    json response;
    response["errno"] = static_cast<int>(code);
    response["errmsg"] = customMessage.empty() ? getErrorMessage(code) : customMessage;
    response["success"] = false;
    return response;
}

json ErrorHandler::createSuccessResponse(const json& data) {
    json response;
    response["errno"] = static_cast<int>(ErrorCode::SUCCESS);
    response["errmsg"] = getErrorMessage(ErrorCode::SUCCESS);
    response["success"] = true;
    if (!data.empty()) {
        response["data"] = data;
    }
    return response;
}

void ErrorHandler::logError(ErrorCode code, const string& context, const string& details) {
    string errorMsg = getErrorMessage(code);
    string logMessage = "[" + to_string(static_cast<int>(code)) + "] " + errorMsg + " - Context: " + context;
    if (!details.empty()) {
        logMessage += " - Details: " + details;
    }
    LOG_ERROR << logMessage;
}

bool ErrorHandler::isSuccess(ErrorCode code) {
    return code == ErrorCode::SUCCESS;
}