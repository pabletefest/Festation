#pragma once

#include <string>
#include <format>

#define LOG_DEBUG(msg, ...) Logger::logDebug(msg, __VA_ARGS__)
#define LOG_INFO(msg, ...) Logger::logInfo(msg, __VA_ARGS__)
#define LOG_WARN(msg, ...) Logger::logWarn(msg, __VA_ARGS__)
#define LOG_ERROR(msg, ...) Logger::logError(msg, __VA_ARGS__)
#define LOG_CRITICAL(msg, ...) Logger::logCritical(msg, __VA_ARGS__)

class Logger {
public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL
    };

    static void log(LogLevel level, const std::string& message);
    //static Logger& getLogger();
    //static void setLogLevel(LogLevel level);

    template<class... Args>
    static void logDebug(const std::string& message, Args&&... args)
    {
        Logger::log(LogLevel::DEBUG, std::vformat(message, std::make_format_args(args...)));
    }
    
    template<class... Args>
    static void logInfo(const std::string& message, Args&&... args)
    {
        Logger::log(LogLevel::INFO, std::vformat(message, std::make_format_args(args...)));
    }

    template<class... Args>
    static void logWarn(const std::string& message, Args&&... args)
    {
        Logger::log(LogLevel::WARN, std::vformat(message, std::make_format_args(args...)));
    }

    template<class... Args>
    static void logError(const std::string& message, Args&&... args)
    {
        Logger::log(LogLevel::ERROR, std::vformat(message, std::make_format_args(args...)));
    }

    template<class... Args>
    static void logCritical(const std::string& message, Args&&... args)
    {
        Logger::log(LogLevel::CRITICAL, std::vformat(message, std::make_format_args(args...)));
    }
};
