#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include "UserModel.hpp"
#include "user.hpp"
#include "../include/server/common/ErrorCodes.hpp"

class PerformanceTest {
public:
    static void testConnectionPool(int threadCount, int operationsPerThread) {
        std::cout << "\n=== 连接池测试 (线程:" << threadCount << ", 操作:" << operationsPerThread << ") ===\n";
        UserModel::setConnectionType(DBConnectionType::CONNECTION_POOL);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        std::atomic<int> successCount(0);
        std::atomic<int> failCount(0);
        
        for (int i = 0; i < threadCount; ++i) {
            threads.emplace_back([&, i]() {
                for (int j = 0; j < operationsPerThread; ++j) {
                    auto result = UserModel().query(23); // 使用存在的用户ID
                    User user = result.first;
                    ErrorCode error = result.second;
                    if (error == ErrorCode::SUCCESS && user.getId() != 0) {
                        successCount++;
                    } else {
                        failCount++;
                    }
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // 简洁的输出格式
        std::cout << "结果: 成功=" << successCount.load() 
                  << ", 失败=" << failCount.load()
                  << ", 耗时=" << duration.count() << "ms"
                  << ", QPS=" << (double)(threadCount * operationsPerThread) / duration.count() * 1000 
                  << std::endl;
    }
};

int main() {
    std::cout << "数据库性能测试\n";
    std::cout << "================\n";
    
    // 简化测试配置
    int threadCount = 2;
    int operationsPerThread = 5;
    
    PerformanceTest::testConnectionPool(threadCount, operationsPerThread);
    
    return 0;
}