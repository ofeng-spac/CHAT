#ifndef PASSWORDUTILS_HPP
#define PASSWORDUTILS_HPP

#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/evp.h>

using namespace std;

/**
 * 密码安全工具类
 * 提供密码哈希、验证、盐值生成等安全功能
 */
class PasswordUtils {
public:
    /**
     * 生成随机盐值
     * @return 16字节的随机盐值（十六进制字符串）
     */
    static string generateSalt();
    
    /**
     * 使用SHA-256算法对密码进行哈希加密
     * @param password 原始密码
     * @param salt 盐值
     * @return 哈希后的密码（十六进制字符串）
     */
    static string hashPassword(const string& password, const string& salt);
    
    /**
     * 验证密码是否正确
     * @param password 用户输入的密码
     * @param hash 存储的哈希值
     * @param salt 存储的盐值
     * @return 密码是否匹配
     */
    static bool verifyPassword(const string& password, const string& hash, const string& salt);
    
    /**
     * 生成包含盐值的完整哈希字符串
     * 格式：salt$hash
     * @param password 原始密码
     * @return 包含盐值的完整哈希字符串
     */
    static string generatePasswordHash(const string& password);
    
    /**
     * 验证密码（使用完整哈希字符串）
     * @param password 用户输入的密码
     * @param storedHash 存储的完整哈希字符串（salt$hash格式）
     * @return 密码是否匹配
     */
    static bool verifyPasswordHash(const string& password, const string& storedHash);
    
private:
    /**
     * 将字节数组转换为十六进制字符串
     * @param data 字节数组
     * @param length 数组长度
     * @return 十六进制字符串
     */
    static string bytesToHex(const unsigned char* data, size_t length);
};

#endif // PASSWORDUTILS_HPP