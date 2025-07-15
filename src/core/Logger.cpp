#include "Logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;
bool Logger::initialized_ = false;

void Logger::initialize() {
    if (!initialized_) {
        // Create a colored console logger
        logger_ = spdlog::stdout_color_mt("PortAudioEngine");
        
        // Set default log level to info
        logger_->set_level(spdlog::level::info);
        
        // Set pattern to include timestamp, level, and message
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        
        initialized_ = true;
        
        logger_->info("Logger initialized with spdlog");
    }
}

void Logger::setLevel(spdlog::level::level_enum level) {
    if (!initialized_) {
        initialize();
    }
    if (logger_) {
        logger_->set_level(level);
    }
}

spdlog::level::level_enum Logger::getLevel() {
    if (!initialized_) {
        initialize();
    }
    if (logger_) {
        return logger_->level();
    }
    return spdlog::level::info;
}

spdlog::level::level_enum Logger::convertLogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::NONE: return spdlog::level::off;
        case LogLevel::ERROR: return spdlog::level::err;
        case LogLevel::WARN: return spdlog::level::warn;
        case LogLevel::INFO: return spdlog::level::info;
        case LogLevel::DEBUG: return spdlog::level::debug;
        case LogLevel::TRACE: return spdlog::level::trace;
        default: return spdlog::level::info;
    }
}

void Logger::setLevel(LogLevel level) {
    setLevel(convertLogLevel(level));
}
