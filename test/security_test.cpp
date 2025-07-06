#include <gtest/gtest.h>
#include "../include/server/common/InputValidator.hpp"
#include "../include/server/model/SecureUserModel.hpp"
#include "../include/server/security/PasswordUtils.hpp"
#include "../include/server/db/SecureDB.hpp"
#include <string>
#include <vector>

using namespace std;

/**
 * 输入验证测试类
 */
class InputValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的设置
    }
    
    void TearDown() override {
        // 测试后的清理
    }
};

/**
 * SQL注入防护测试类
 */
class SQLInjectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化测试数据库连接
        secureUserModel = std::make_unique<SecureUserModel>();
    }
    
    void TearDown() override {
        // 清理测试数据
    }
    
    std::unique_ptr<SecureUserModel> secureUserModel;
};

// ==================== 输入验证测试 ====================

/**
 * 测试用户名验证功能
 */
TEST_F(InputValidatorTest, TestUsernameValidation) {
    // 测试有效用户名
    EXPECT_EQ(InputValidator::validateUsername("validuser"), ErrorCode::SUCCESS);
    EXPECT_EQ(InputValidator::validateUsername("user123"), ErrorCode::SUCCESS);
    EXPECT_EQ(InputValidator::validateUsername("user_name"), ErrorCode::SUCCESS);
    
    // 测试无效用户名 - 太短
    EXPECT_EQ(InputValidator::validateUsername("ab"), ErrorCode::USERNAME_TOO_SHORT);
    
    // 测试无效用户名 - 太长
    string longUsername(25, 'a');
    EXPECT_EQ(InputValidator::validateUsername(longUsername), ErrorCode::USERNAME_TOO_LONG);
    
    // 测试无效字符
    EXPECT_EQ(InputValidator::validateUsername("user@name"), ErrorCode::INVALID_USERNAME);
    EXPECT_EQ(InputValidator::validateUsername("user name"), ErrorCode::INVALID_USERNAME);
    EXPECT_EQ(InputValidator::validateUsername("user-name"), ErrorCode::INVALID_USERNAME);
}

/**
 * 测试密码验证功能
 */
TEST_F(InputValidatorTest, TestPasswordValidation) {
    // 测试有效密码
    EXPECT_EQ(InputValidator::validatePassword("password123"), ErrorCode::SUCCESS);
    EXPECT_EQ(InputValidator::validatePassword("MyP@ssw0rd!"), ErrorCode::SUCCESS);
    
    // 测试无效密码 - 太短
    EXPECT_EQ(InputValidator::validatePassword("12345"), ErrorCode::PASSWORD_TOO_SHORT);
    
    // 测试无效密码 - 太长
    string longPassword(55, 'a');
    EXPECT_EQ(InputValidator::validatePassword(longPassword), ErrorCode::PASSWORD_TOO_LONG);
}

/**
 * 测试消息验证功能
 */
TEST_F(InputValidatorTest, TestMessageValidation) {
    // 测试有效消息
    EXPECT_EQ(InputValidator::validateMessage("Hello, world!"), ErrorCode::SUCCESS);
    EXPECT_EQ(InputValidator::validateMessage("你好，世界！"), ErrorCode::SUCCESS);
    
    // 测试空消息
    EXPECT_EQ(InputValidator::validateMessage(""), ErrorCode::MESSAGE_EMPTY);
    
    // 测试过长消息
    string longMessage(1001, 'a');
    EXPECT_EQ(InputValidator::validateMessage(longMessage), ErrorCode::MESSAGE_TOO_LONG);
}

/**
 * 测试危险字符检测
 */
TEST_F(InputValidatorTest, TestDangerousCharacterDetection) {
    // 测试SQL注入相关字符
    EXPECT_TRUE(InputValidator::containsDangerousChars("'; DROP TABLE users; --"));
    EXPECT_TRUE(InputValidator::containsDangerousChars("admin' OR '1'='1"));
    EXPECT_TRUE(InputValidator::containsDangerousChars("user\"name"));
    EXPECT_TRUE(InputValidator::containsDangerousChars("user/*comment*/name"));
    
    // 测试XSS相关字符
    EXPECT_TRUE(InputValidator::containsDangerousChars("<script>alert('xss')</script>"));
    EXPECT_TRUE(InputValidator::containsDangerousChars("<img src=x onerror=alert(1)>"));
    
    // 测试正常字符
    EXPECT_FALSE(InputValidator::containsDangerousChars("normaluser"));
    EXPECT_FALSE(InputValidator::containsDangerousChars("user_123"));
    EXPECT_FALSE(InputValidator::containsDangerousChars("Hello World!"));
}

/**
 * 测试字符串清理功能
 */
TEST_F(InputValidatorTest, TestStringSanitization) {
    // 测试SQL注入字符清理
    string maliciousInput = "admin'; DROP TABLE users; --";
    string sanitized = InputValidator::sanitizeString(maliciousInput);
    EXPECT_EQ(sanitized.find("'"), string::npos);
    EXPECT_EQ(sanitized.find(";"), string::npos);
    EXPECT_EQ(sanitized.find("--"), string::npos);
    
    // 测试正常字符串不被修改
    string normalInput = "normaluser123";
    string normalSanitized = InputValidator::sanitizeString(normalInput);
    EXPECT_EQ(normalInput, normalSanitized);
}

/**
 * 测试JSON格式验证
 */
TEST_F(InputValidatorTest, TestJSONValidation) {
    // 测试有效JSON
    EXPECT_EQ(InputValidator::validateJsonFormat("{\"name\":\"test\"}"), ErrorCode::SUCCESS);
    EXPECT_EQ(InputValidator::validateJsonFormat("[1,2,3]"), ErrorCode::SUCCESS);
    
    // 测试无效JSON
    EXPECT_EQ(InputValidator::validateJsonFormat("{name:test}"), ErrorCode::INVALID_JSON_FORMAT);
    EXPECT_EQ(InputValidator::validateJsonFormat("invalid json"), ErrorCode::INVALID_JSON_FORMAT);
}

// ==================== SQL注入防护测试 ====================

/**
 * 测试用户注册的SQL注入防护
 */
TEST_F(SQLInjectionTest, TestRegisterSQLInjectionProtection) {
    // 测试恶意用户名注入
    User maliciousUser;
    maliciousUser.setName("admin'; DROP TABLE user; --");
    maliciousUser.setPwd("password123");
    maliciousUser.setState("online");
    
    // 应该返回验证错误，而不是执行恶意SQL
    ErrorCode result = secureUserModel->insert(maliciousUser);
    EXPECT_NE(result, ErrorCode::SUCCESS);
    EXPECT_EQ(result, ErrorCode::INVALID_USERNAME);
}

/**
 * 测试用户查询的SQL注入防护
 */
TEST_F(SQLInjectionTest, TestQuerySQLInjectionProtection) {
    // 测试恶意用户名查询
    string maliciousName = "admin' OR '1'='1";
    auto [user, result] = secureUserModel->queryByName(maliciousName);
    
    // 应该返回验证错误或用户不存在
    EXPECT_NE(result, ErrorCode::SUCCESS);
}

/**
 * 测试登录的SQL注入防护
 */
TEST_F(SQLInjectionTest, TestLoginSQLInjectionProtection) {
    // 测试恶意登录尝试
    string maliciousUsername = "admin' --";
    string password = "anypassword";
    
    auto [user, result] = secureUserModel->login(maliciousUsername, password);
    
    // 应该返回验证错误
    EXPECT_NE(result, ErrorCode::SUCCESS);
    EXPECT_EQ(result, ErrorCode::INVALID_USERNAME);
}

/**
 * 测试状态更新的SQL注入防护
 */
TEST_F(SQLInjectionTest, TestUpdateStateSQLInjectionProtection) {
    // 测试恶意状态值
    int userId = 1;
    string maliciousState = "online'; UPDATE user SET name='hacked' WHERE '1'='1";
    
    ErrorCode result = secureUserModel->updateState(userId, maliciousState);
    
    // 应该返回验证错误
    EXPECT_NE(result, ErrorCode::SUCCESS);
    EXPECT_EQ(result, ErrorCode::INVALID_INPUT);
}

// ==================== 密码安全测试 ====================

/**
 * 测试密码哈希功能
 */
TEST(PasswordSecurityTest, TestPasswordHashing) {
    string password = "testpassword123";
    string salt = PasswordUtils::generateSalt();
    
    // 测试哈希生成
    string hash1 = PasswordUtils::hashPassword(password, salt);
    string hash2 = PasswordUtils::hashPassword(password, salt);
    
    // 相同密码和盐应该产生相同哈希
    EXPECT_EQ(hash1, hash2);
    
    // 不同盐应该产生不同哈希
    string differentSalt = PasswordUtils::generateSalt();
    string hash3 = PasswordUtils::hashPassword(password, differentSalt);
    EXPECT_NE(hash1, hash3);
}

/**
 * 测试盐值生成
 */
TEST(PasswordSecurityTest, TestSaltGeneration) {
    string salt1 = PasswordUtils::generateSalt();
    string salt2 = PasswordUtils::generateSalt();
    
    // 每次生成的盐值应该不同
    EXPECT_NE(salt1, salt2);
    
    // 盐值长度应该合理
    EXPECT_GT(salt1.length(), 0);
    EXPECT_GT(salt2.length(), 0);
}

// ==================== 数据库安全测试 ====================

/**
 * 测试数据库连接安全
 */
TEST(DatabaseSecurityTest, TestSecureDatabaseConnection) {
    SecureDB db;
    
    // 测试连接
    bool connected = db.connect();
    
    if (connected) {
        // 测试转义功能
        string maliciousInput = "'; DROP TABLE test; --";
        string escaped = db.escapeString(maliciousInput);
        
        // 转义后应该不包含危险字符
        EXPECT_NE(maliciousInput, escaped);
        
        db.close();
    }
}

/**
 * 测试预编译语句
 */
TEST(DatabaseSecurityTest, TestPreparedStatements) {
    SecureDB db;
    
    if (db.connect()) {
        // 测试带参数的查询
        string sql = "SELECT COUNT(*) FROM user WHERE name = ?";
        vector<string> params = {"testuser"};
        
        MYSQL_RES* result = db.executeQuery(sql, params);
        
        // 查询应该成功执行（即使用户不存在）
        EXPECT_NE(result, nullptr);
        
        if (result) {
            mysql_free_result(result);
        }
        
        db.close();
    }
}

// ==================== 集成测试 ====================

/**
 * 测试完整的用户注册流程安全性
 */
TEST_F(SQLInjectionTest, TestCompleteRegistrationSecurity) {
    // 创建一个正常用户
    User normalUser;
    normalUser.setName("testuser123");
    normalUser.setPwd("securepassword");
    normalUser.setState("offline");
    
    // 注册应该成功
    ErrorCode result = secureUserModel->insert(normalUser);
    
    if (result == ErrorCode::SUCCESS) {
        // 验证用户确实被插入
        auto [retrievedUser, queryResult] = secureUserModel->queryByName("testuser123");
        EXPECT_EQ(queryResult, ErrorCode::SUCCESS);
        EXPECT_EQ(retrievedUser.getName(), "testuser123");
        
        // 清理测试数据
        secureUserModel->deleteUser(retrievedUser.getId());
    }
}

/**
 * 测试登录流程的安全性
 */
TEST_F(SQLInjectionTest, TestCompleteLoginSecurity) {
    // 首先注册一个用户
    User testUser;
    testUser.setName("logintest");
    testUser.setPwd("testpassword");
    testUser.setState("offline");
    
    ErrorCode registerResult = secureUserModel->insert(testUser);
    
    if (registerResult == ErrorCode::SUCCESS) {
        // 测试正确登录
        auto [user1, result1] = secureUserModel->login("logintest", "testpassword");
        EXPECT_EQ(result1, ErrorCode::SUCCESS);
        
        // 测试错误密码
        auto [user2, result2] = secureUserModel->login("logintest", "wrongpassword");
        EXPECT_EQ(result2, ErrorCode::INVALID_PASSWORD);
        
        // 测试恶意登录尝试
        auto [user3, result3] = secureUserModel->login("logintest' --", "anypassword");
        EXPECT_NE(result3, ErrorCode::SUCCESS);
        
        // 清理测试数据
        secureUserModel->deleteUser(testUser.getId());
    }
}

// 主函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}