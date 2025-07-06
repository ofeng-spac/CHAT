#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <chrono>
#include <iomanip>

using namespace std;

// 日志级别枚举
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
};

// 日志配置结构
struct LogConfig {
    LogLevel level = LogLevel::INFO;           // 最低日志级别
    string logDir = "./logs";                  // 日志目录
    string logFileName = "chat_server.log";    // 日志文件名
    bool enableConsole = true;                 // 是否输出到控制台
    bool enableFile = true;                    // 是否输出到文件
    size_t maxFileSize = 10 * 1024 * 1024;     // 最大文件大小(10MB)
    int maxFileCount = 5;                      // 最大文件数量
    bool enableRotation = true;                // 是否启用日志轮转
};

// 单例日志类
class Logger {
public:
    static Logger& getInstance();
    
    // 初始化日志系统
    void init(const LogConfig& config = LogConfig());
    
    // 记录日志
    void log(LogLevel level, const string& message, const string& file = "", int line = 0);
    
    // 便捷方法
    void debug(const string& message, const string& file = "", int line = 0);
    void info(const string& message, const string& file = "", int line = 0);
    void warn(const string& message, const string& file = "", int line = 0);
    void error(const string& message, const string& file = "", int line = 0);
    void fatal(const string& message, const string& file = "", int line = 0);
    
    // 设置日志级别
    void setLogLevel(LogLevel level);
    
    // 获取当前日志级别
    LogLevel getLogLevel() const;
    
    // 刷新日志缓冲区
    void flush();
    
    // 关闭日志系统
    void shutdown();
    
private:
    Logger() = default;
    ~Logger();
    
    // 禁用拷贝构造和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // 内部方法
    string formatMessage(LogLevel level, const string& message, const string& file, int line);
    string getCurrentTime();
    string levelToString(LogLevel level);
    void writeToFile(const string& message);
    void writeToConsole(const string& message, LogLevel level);
    void rotateLogFile();
    bool shouldLog(LogLevel level) const;
    void ensureLogDirectory();
    
private:
    LogConfig config_;
    unique_ptr<ofstream> fileStream_;
    mutex logMutex_;
    bool initialized_ = false;
    size_t currentFileSize_ = 0;
};

// 日志宏定义
#define CHAT_LOG_DEBUG(msg) Logger::getInstance().debug(msg, __FILE__, __LINE__)
#define CHAT_LOG_INFO(msg) Logger::getInstance().info(msg, __FILE__, __LINE__)
#define CHAT_LOG_WARN(msg) Logger::getInstance().warn(msg, __FILE__, __LINE__)
#define CHAT_LOG_ERROR(msg) Logger::getInstance().error(msg, __FILE__, __LINE__)
#define CHAT_LOG_FATAL(msg) Logger::getInstance().fatal(msg, __FILE__, __LINE__)

// 格式化日志宏
#define CHAT_LOG_DEBUG_F(fmt, ...) do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
    Logger::getInstance().debug(buffer, __FILE__, __LINE__); \
} while(0)

#define CHAT_LOG_INFO_F(fmt, ...) do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
    Logger::getInstance().info(buffer, __FILE__, __LINE__); \
} while(0)

#define CHAT_LOG_WARN_F(fmt, ...) do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
    Logger::getInstance().warn(buffer, __FILE__, __LINE__); \
} while(0)

#define CHAT_LOG_ERROR_F(fmt, ...) do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
    Logger::getInstance().error(buffer, __FILE__, __LINE__); \
} while(0)

#define CHAT_LOG_FATAL_F(fmt, ...) do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
    Logger::getInstance().fatal(buffer, __FILE__, __LINE__); \
} while(0)

// 性能日志类
class PerformanceLogger {
public:
    PerformanceLogger(const string& operation);
    ~PerformanceLogger();
    
private:
    string operation_;
    chrono::high_resolution_clock::time_point startTime_;
};

// 性能监控宏
#define PERF_LOG(operation) PerformanceLogger _perf_logger(operation)

#endif // LOGGER_HPP