#ifndef ENHANCEDINPUTVALIDATOR_HPP
#define ENHANCEDINPUTVALIDATOR_HPP

#include "InputValidator.hpp"
#include <string>
#include <vector>
#include <regex>
#include <unordered_set>

using namespace std;

/**
 * 增强的输入验证器
 * 提供更强的SQL注入防护和输入验证功能
 */
class EnhancedInputValidator : public InputValidator {
public:
    /**
     * 高级SQL注入检测
     * @param input 输入字符串
     * @return 是否包含SQL注入攻击
     */
    static bool containsSQLInjection(const string& input);
    
    /**
     * 检测XSS攻击
     * @param input 输入字符串
     * @return 是否包含XSS攻击
     */
    static bool containsXSS(const string& input);
    
    /**
     * 检测命令注入
     * @param input 输入字符串
     * @return 是否包含命令注入
     */
    static bool containsCommandInjection(const string& input);
    
    /**
     * 高级字符串清理
     * @param input 输入字符串
     * @param allowHtml 是否允许HTML标签
     * @return 清理后的字符串
     */
    static string advancedSanitize(const string& input, bool allowHtml = false);
    
    /**
     * 验证SQL查询参数
     * @param param 参数值
     * @param paramType 参数类型（string, int, email等）
     * @return 验证结果
     */
    static ErrorCode validateSQLParameter(const string& param, const string& paramType);
    
    /**
     * 验证邮箱格式
     * @param email 邮箱地址
     * @return 验证结果
     */
    static ErrorCode validateEmail(const string& email);
    
    /**
     * 验证电话号码
     * @param phone 电话号码
     * @return 验证结果
     */
    static ErrorCode validatePhone(const string& phone);
    
    /**
     * 验证IP地址
     * @param ip IP地址
     * @return 验证结果
     */
    static ErrorCode validateIPAddress(const string& ip);
    
    /**
     * 验证URL格式
     * @param url URL地址
     * @return 验证结果
     */
    static ErrorCode validateURL(const string& url);
    
    /**
     * 检查输入长度限制
     * @param input 输入字符串
     * @param maxLength 最大长度
     * @param fieldName 字段名称（用于错误信息）
     * @return 验证结果
     */
    static ErrorCode validateLength(const string& input, size_t maxLength, const string& fieldName);
    
    /**
     * 验证文件名安全性
     * @param filename 文件名
     * @return 验证结果
     */
    static ErrorCode validateFilename(const string& filename);
    
    /**
     * 验证路径安全性（防止目录遍历攻击）
     * @param path 路径
     * @return 验证结果
     */
    static ErrorCode validatePath(const string& path);
    
    /**
     * 白名单验证
     * @param input 输入值
     * @param whitelist 白名单集合
     * @return 是否在白名单中
     */
    static bool isInWhitelist(const string& input, const unordered_set<string>& whitelist);
    
    /**
     * 黑名单验证
     * @param input 输入值
     * @param blacklist 黑名单集合
     * @return 是否在黑名单中
     */
    static bool isInBlacklist(const string& input, const unordered_set<string>& blacklist);
    
    /**
     * 验证密码强度
     * @param password 密码
     * @return 验证结果
     */
    static ErrorCode validatePasswordStrength(const string& password);
    
    /**
     * 检测重复字符攻击
     * @param input 输入字符串
     * @param maxRepeats 最大重复次数
     * @return 是否包含过多重复字符
     */
    static bool containsExcessiveRepeats(const string& input, int maxRepeats = 10);
    
    /**
     * 验证数字范围
     * @param value 数值
     * @param min 最小值
     * @param max 最大值
     * @return 验证结果
     */
    static ErrorCode validateNumberRange(int value, int min, int max);
    
    /**
     * 验证字符串只包含允许的字符
     * @param input 输入字符串
     * @param allowedChars 允许的字符集
     * @return 验证结果
     */
    static ErrorCode validateCharacterSet(const string& input, const string& allowedChars);
    
private:
    // SQL注入关键词列表
    static const vector<string> SQL_INJECTION_KEYWORDS;
    
    // XSS攻击模式
    static const vector<string> XSS_PATTERNS;
    
    // 命令注入关键词
    static const vector<string> COMMAND_INJECTION_KEYWORDS;
    
    // 危险文件扩展名
    static const unordered_set<string> DANGEROUS_EXTENSIONS;
    
    /**
     * 检查是否包含关键词
     * @param input 输入字符串
     * @param keywords 关键词列表
     * @return 是否包含
     */
    static bool containsKeywords(const string& input, const vector<string>& keywords);
    
    /**
     * 检查是否匹配模式
     * @param input 输入字符串
     * @param patterns 模式列表
     * @return 是否匹配
     */
    static bool matchesPatterns(const string& input, const vector<string>& patterns);
    
    /**
     * 转换为小写
     * @param str 输入字符串
     * @return 小写字符串
     */
    static string toLowerCase(const string& str);
    
    /**
     * 移除空白字符
     * @param str 输入字符串
     * @return 去除空白后的字符串
     */
    static string removeWhitespace(const string& str);
};

#endif // ENHANCEDINPUTVALIDATOR_HPP