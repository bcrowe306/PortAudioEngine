#pragma once

#include <iostream>
#include <sstream>

enum class LogLevel {
    NONE = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4
};

class Logger {
public:
    static void setLevel(LogLevel level) {
        currentLevel = level;
    }
    
    static LogLevel getLevel() {
        return currentLevel;
    }
    
    template<typename... Args>
    static void log(LogLevel level, Args&&... args) {
        if (level <= currentLevel) {
            std::ostringstream oss;
            (oss << ... << args);
            std::cout << oss.str() << std::endl;
        }
    }
    
    template<typename... Args>
    static void error(Args&&... args) {
        log(LogLevel::ERROR, "[ERROR] ", args...);
    }
    
    template<typename... Args>
    static void warn(Args&&... args) {
        log(LogLevel::WARN, "[WARN] ", args...);
    }
    
    template<typename... Args>
    static void info(Args&&... args) {
        log(LogLevel::INFO, "[INFO] ", args...);
    }
    
    template<typename... Args>
    static void debug(Args&&... args) {
        log(LogLevel::DEBUG, "[DEBUG] ", args...);
    }

private:
    static LogLevel currentLevel;
};
