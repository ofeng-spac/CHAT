#ifndef ERRORCODES_HPP
#define ERRORCODES_HPP

#include <string>
#include <unordered_map>
#include "../../../thirdparty/json.hpp"

using json = nlohmann::json;
using namespace std;

/**
 * 系统错误码枚举
 * 按模块分类，便于维护和扩展
 */
enum class ErrorCode {
    // 成功
    SUCCESS = 0,
    
    // 用户相关错误 (1000-1999)
    USER_NOT_FOUND = 1001,
    USER_ALREADY_EXISTS = 1002,
    INVALID_PASSWORD = 1003,
    USER_ALREADY_ONLINE = 1004,
    USER_NOT_ONLINE = 1005,
    INVALID_USERNAME = 1006,
    PASSWORD_TOO_SHORT = 1007,
    PASSWORD_TOO_LONG = 1008,
    USERNAME_TOO_SHORT = 1009,
    USERNAME_TOO_LONG = 1010,
    
    // 好友相关错误 (2000-2999)
    FRIEND_NOT_FOUND = 2001,
    FRIEND_ALREADY_EXISTS = 2002,
    CANNOT_ADD_SELF = 2003,
    FRIEND_REQUEST_PENDING = 2004,
    
    // 群组相关错误 (3000-3999)
    GROUP_NOT_FOUND = 3001,
    GROUP_ALREADY_EXISTS = 3002,
    NOT_GROUP_MEMBER = 3003,
    ALREADY_GROUP_MEMBER = 3004,
    INSUFFICIENT_PERMISSION = 3005,
    GROUP_FULL = 3006,
    
    // 消息相关错误 (4000-4999)
    MESSAGE_TOO_LONG = 4001,
    MESSAGE_EMPTY = 4002,
    INVALID_MESSAGE_TYPE = 4003,
    MESSAGE_SEND_FAILED = 4004,
    
    // 数据库相关错误 (5000-5999)
    DATABASE_CONNECTION_FAILED = 5001,
    DATABASE_QUERY_FAILED = 5002,
    DATABASE_UPDATE_FAILED = 5003,
    DATABASE_INSERT_FAILED = 5004,
    DATABASE_DELETE_FAILED = 5005,
    CONNECTION_POOL_EXHAUSTED = 5006,
    
    // 网络相关错误 (6000-6999)
    NETWORK_ERROR = 6001,
    CONNECTION_LOST = 6002,
    INVALID_REQUEST_FORMAT = 6003,
    REQUEST_TIMEOUT = 6004,
    
    // Redis相关错误 (7000-7999)
    REDIS_CONNECTION_FAILED = 7001,
    REDIS_OPERATION_FAILED = 7002,
    
    // 系统相关错误 (8000-8999)
    INTERNAL_SERVER_ERROR = 8001,
    SERVICE_UNAVAILABLE = 8002,
    INVALID_CONFIGURATION = 8003,
    RESOURCE_EXHAUSTED = 8004,
    
    // 输入验证错误 (9000-9999)
    INVALID_INPUT = 9001,
    MISSING_REQUIRED_FIELD = 9002,
    INVALID_JSON_FORMAT = 9003,
    PARAMETER_OUT_OF_RANGE = 9004,
    INPUT_VALIDATION_EMPTY_FIELD = 9005,
    INPUT_VALIDATION_SQL_INJECTION = 9006,
    INPUT_VALIDATION_XSS_ATTACK = 9007,
    INPUT_VALIDATION_COMMAND_INJECTION = 9008,
    INPUT_VALIDATION_INVALID_FORMAT = 9009,
    INPUT_VALIDATION_MALICIOUS_INPUT = 9010,
    INPUT_VALIDATION_TOO_LONG = 9011,
    INPUT_VALIDATION_INVALID_CHARACTERS = 9012,
    INPUT_VALIDATION_DANGEROUS_FILE_TYPE = 9013,
    INPUT_VALIDATION_RESERVED_NAME = 9014,
    INPUT_VALIDATION_PATH_TRAVERSAL = 9015,
    INPUT_VALIDATION_ABSOLUTE_PATH = 9016,
    INPUT_VALIDATION_PASSWORD_TOO_SHORT = 9017,
    INPUT_VALIDATION_PASSWORD_TOO_LONG = 9018,
    INPUT_VALIDATION_PASSWORD_TOO_WEAK = 9019,
    INPUT_VALIDATION_PASSWORD_TOO_COMMON = 9020,
    INPUT_VALIDATION_OUT_OF_RANGE = 9021
};

/**
 * 错误处理工具类
 * 提供错误码到消息的映射、错误响应生成等功能
 */
class ErrorHandler {
public:
    /**
     * 获取错误码对应的错误消息
     * @param code 错误码
     * @return 错误消息字符串
     */
    static string getErrorMessage(ErrorCode code);
    
    /**
     * 创建标准错误响应JSON
     * @param code 错误码
     * @param customMessage 自定义错误消息（可选）
     * @return JSON格式的错误响应
     */
    static json createErrorResponse(ErrorCode code, const string& customMessage = "");
    
    /**
     * 创建成功响应JSON
     * @param data 响应数据（可选）
     * @return JSON格式的成功响应
     */
    static json createSuccessResponse(const json& data = json::object());
    
    /**
     * 记录错误日志
     * @param code 错误码
     * @param context 错误上下文信息
     * @param details 详细错误信息（可选）
     */
    static void logError(ErrorCode code, const string& context, const string& details = "");
    
    /**
     * 检查错误码是否为成功
     * @param code 错误码
     * @return 是否成功
     */
    static bool isSuccess(ErrorCode code);
    
private:
    // 错误码到消息的映射表
    static unordered_map<ErrorCode, string> errorMessages;
    
    // 初始化错误消息映射表
    static void initErrorMessages();
    
    // 确保错误消息映射表已初始化
    static void ensureInitialized();
};

#endif // ERRORCODES_HPP