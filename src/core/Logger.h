#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <memory>
#include <string>

class Logger {
public:
    // Initialize the logger (call this at startup)
    static void initialize();
    
    // Set log level
    static void setLevel(spdlog::level::level_enum level);
    
    // Get the current log level
    static spdlog::level::level_enum getLevel();
    
    // Logging methods using spdlog with format strings
    template<typename... Args>
    static void error(const std::string &fmt, Args&&... args) {
        if (logger_) {
            logger_->error(SPDLOG_FMT_RUNTIME(fmt), std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    static void warn(const std::string &fmt, Args&&... args) {
        if (logger_) {
            logger_->warn(SPDLOG_FMT_RUNTIME(fmt), std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    static void info(const std::string &fmt, Args&&... args) {
        if (logger_) {
            logger_->info(SPDLOG_FMT_RUNTIME(fmt), std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    static void debug(const std::string &fmt, Args&&... args) {
        if (logger_) {
            logger_->debug(SPDLOG_FMT_RUNTIME(fmt), std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    static void trace(const std::string &fmt, Args&&... args) {
        if (logger_) {
            logger_->trace(SPDLOG_FMT_RUNTIME(fmt), std::forward<Args>(args)...);
        }
    }
    
    // Simple message logging without format strings (for single string messages)
    static void error(const std::string& msg) {
        if (logger_) {
            logger_->error(msg);
        }
    }
    
    static void warn(const std::string& msg) {
        if (logger_) {
            logger_->warn(msg);
        }
    }
    
    static void info(const std::string& msg) {
        if (logger_) {
            logger_->info(msg);
        }
    }
    
    static void debug(const std::string& msg) {
        if (logger_) {
            logger_->debug(msg);
        }
    }
    
    static void trace(const std::string& msg) {
        if (logger_) {
            logger_->trace(msg);
        }
    }
    
    // For backward compatibility with old log level enum
    enum class LogLevel {
        NONE = 0,
        ERROR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4,
        TRACE = 5
    };
    
    // Convert old LogLevel to spdlog level
    static spdlog::level::level_enum convertLogLevel(LogLevel level);
    
    // Legacy method for compatibility
    static void setLevel(LogLevel level);

private:
    static std::shared_ptr<spdlog::logger> logger_;
    static bool initialized_;
};
