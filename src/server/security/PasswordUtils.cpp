#include "server/security/PasswordUtils.hpp"
#include <muduo/base/Logging.h>

string PasswordUtils::generateSalt() {
    // 生成16字节的随机盐值
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 255);
    
    unsigned char salt[16];
    for (int i = 0; i < 16; ++i) {
        salt[i] = static_cast<unsigned char>(dis(gen));
    }
    
    return bytesToHex(salt, 16);
}

string PasswordUtils::hashPassword(const string& password, const string& salt) {
    // 将密码和盐值组合
    string combined = password + salt;
    
    // 使用SHA-256进行哈希
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, combined.c_str(), combined.length());
    SHA256_Final(hash, &sha256);
    
    return bytesToHex(hash, SHA256_DIGEST_LENGTH);
}

bool PasswordUtils::verifyPassword(const string& password, const string& hash, const string& salt) {
    string computedHash = hashPassword(password, salt);
    return computedHash == hash;
}

string PasswordUtils::generatePasswordHash(const string& password) {
    string salt = generateSalt();
    string hash = hashPassword(password, salt);
    return salt + "$" + hash;
}

bool PasswordUtils::verifyPasswordHash(const string& password, const string& storedHash) {
    // 解析存储的哈希字符串
    size_t dollarPos = storedHash.find('$');
    if (dollarPos == string::npos) {
        LOG_ERROR << "Invalid stored hash format: " << storedHash;
        return false;
    }
    
    string salt = storedHash.substr(0, dollarPos);
    string hash = storedHash.substr(dollarPos + 1);
    
    return verifyPassword(password, hash, salt);
}

string PasswordUtils::bytesToHex(const unsigned char* data, size_t length) {
    stringstream ss;
    ss << hex << setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}