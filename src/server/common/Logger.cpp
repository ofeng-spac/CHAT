#include "server/common/Logger.hpp"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    shutdown();
}

void Logger::init(const LogConfig& config) {
    lock_guard<mutex> lock(logMutex_);
    
    config_ = config;
    
    if (config_.enableFile) {
        ensureLogDirectory();
        
        string fullPath = config_.logDir + "/" + config_.logFileName;
        fileStream_.reset(new ofstream(fullPath, ios::app));
        
        if (!fileStream_->is_open()) {
            cerr << "Failed to open log file: " << fullPath << endl;
            config_.enableFile = false;
        } else {
            // 获取当前文件大小
            fileStream_->seekp(0, ios::end);
            currentFileSize_ = fileStream_->tellp();
        }
    }
    
    initialized_ = true;
    
    // 记录初始化信息
    info("Logger initialized successfully");
    info("Log level: " + levelToString(config_.level));
    info("Console output: " + string(config_.enableConsole ? "enabled" : "disabled"));
    info("File output: " + string(config_.enableFile ? "enabled" : "disabled"));
}

void Logger::log(LogLevel level, const string& message, const string& file, int line) {
    if (!shouldLog(level)) {
        return;
    }
    
    lock_guard<mutex> lock(logMutex_);
    
    string formattedMessage = formatMessage(level, message, file, line);
    
    if (config_.enableConsole) {
        writeToConsole(formattedMessage, level);
    }
    
    if (config_.enableFile && fileStream_ && fileStream_->is_open()) {
        writeToFile(formattedMessage);
        
        // 检查是否需要轮转日志
        if (config_.enableRotation && currentFileSize_ > config_.maxFileSize) {
            rotateLogFile();
        }
    }
}

void Logger::debug(const string& message, const string& file, int line) {
    log(LogLevel::DEBUG, message, file, line);
}

void Logger::info(const string& message, const string& file, int line) {
    log(LogLevel::INFO, message, file, line);
}

void Logger::warn(const string& message, const string& file, int line) {
    log(LogLevel::WARN, message, file, line);
}

void Logger::error(const string& message, const string& file, int line) {
    log(LogLevel::ERROR, message, file, line);
}

void Logger::fatal(const string& message, const string& file, int line) {
    log(LogLevel::FATAL, message, file, line);
    flush();
}

void Logger::setLogLevel(LogLevel level) {
    lock_guard<mutex> lock(logMutex_);
    config_.level = level;
}

LogLevel Logger::getLogLevel() const {
    return config_.level;
}

void Logger::flush() {
    lock_guard<mutex> lock(logMutex_);
    
    if (config_.enableConsole) {
        cout.flush();
        cerr.flush();
    }
    
    if (config_.enableFile && fileStream_) {
        fileStream_->flush();
    }
}

void Logger::shutdown() {
    lock_guard<mutex> lock(logMutex_);
    
    if (initialized_) {
        info("Logger shutting down");
        
        if (fileStream_) {
            fileStream_->flush();
            fileStream_->close();
            fileStream_.reset();
        }
        
        initialized_ = false;
    }
}

string Logger::formatMessage(LogLevel level, const string& message, const string& file, int line) {
    ostringstream oss;
    
    // 时间戳
    oss << "[" << getCurrentTime() << "]";
    
    // 日志级别
    oss << " [" << levelToString(level) << "]";
    
    // 线程ID
    oss << " [" << std::this_thread::get_id() << "]";
    
    // 文件和行号（如果提供）
    if (!file.empty() && line > 0) {
        // 只显示文件名，不显示完整路径
        size_t pos = file.find_last_of("/\\");
        string filename = (pos != string::npos) ? file.substr(pos + 1) : file;
        oss << " [" << filename << ":" << line << "]";
    }
    
    // 消息内容
    oss << " " << message;
    
    return oss.str();
}

string Logger::getCurrentTime() {
    auto now = chrono::system_clock::now();
    auto time_t = chrono::system_clock::to_time_t(now);
    auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    ostringstream oss;
    oss << put_time(localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << setfill('0') << setw(3) << ms.count();
    
    return oss.str();
}

string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void Logger::writeToFile(const string& message) {
    if (fileStream_ && fileStream_->is_open()) {
        *fileStream_ << message << endl;
        currentFileSize_ += message.length() + 1; // +1 for newline
    }
}

void Logger::writeToConsole(const string& message, LogLevel level) {
    // 根据日志级别选择输出流
    if (level >= LogLevel::ERROR) {
        cerr << message << endl;
    } else {
        cout << message << endl;
    }
}

void Logger::rotateLogFile() {
    if (!fileStream_) return;
    
    fileStream_->close();
    
    // 轮转现有日志文件
    string basePath = config_.logDir + "/" + config_.logFileName;
    
    // 删除最老的日志文件
    string oldestFile = basePath + "." + to_string(config_.maxFileCount - 1);
    struct stat st;
        if (stat(oldestFile.c_str(), &st) == 0) {
            remove(oldestFile.c_str());
    }
    
    // 重命名现有文件
    for (int i = config_.maxFileCount - 2; i >= 0; i--) {
        string currentFile = (i == 0) ? basePath : basePath + "." + to_string(i);
        string nextFile = basePath + "." + to_string(i + 1);
        
        struct stat st;
        if (stat(currentFile.c_str(), &st) == 0) {
            rename(currentFile.c_str(), nextFile.c_str());
        }
    }
    
    // 创建新的日志文件
    fileStream_.reset(new ofstream(basePath, ios::app));
    currentFileSize_ = 0;
    
    if (fileStream_->is_open()) {
        info("Log file rotated successfully");
    } else {
        cerr << "Failed to create new log file after rotation" << endl;
        config_.enableFile = false;
    }
}

bool Logger::shouldLog(LogLevel level) const {
    return initialized_ && level >= config_.level;
}

void Logger::ensureLogDirectory() {
    struct stat st;
    memset(&st, 0, sizeof(st));
    if (stat(config_.logDir.c_str(), &st) == -1) {
        if (mkdir(config_.logDir.c_str(), 0755) != 0) {
            cerr << "Failed to create log directory" << endl;
            config_.enableFile = false;
        }
    }
}

// PerformanceLogger 实现
PerformanceLogger::PerformanceLogger(const string& operation) 
    : operation_(operation), startTime_(chrono::high_resolution_clock::now()) {
    CHAT_LOG_DEBUG("Performance monitoring started for: " + operation_);
}

PerformanceLogger::~PerformanceLogger() {
    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime_);
    
    ostringstream oss;
    oss << "Performance: " << operation_ << " took " << duration.count() << " microseconds";
    
    Logger::getInstance().info(oss.str());
}