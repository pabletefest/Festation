#pragma once

#include <string>
#include <format>

#define LOG_DEBUG(...) ::festation::Logger::logDebug(__VA_ARGS__)
#define LOG_INFO(...) ::festation::Logger::logInfo(__VA_ARGS__)
#define LOG_WARN(...) ::festation::Logger::logWarn(__VA_ARGS__)
#define LOG_ERROR(...) ::festation::Logger::logError(__VA_ARGS__)
#define LOG_CRITICAL(...) ::festation::Logger::logCritical(__VA_ARGS__)

#define LOG_KERNEL(...) ::festation::Logger::logKernel(__VA_ARGS__)

namespace festation {
    class Logger {
    public:
        enum class LogLevel {
            DEBUG,
            INFO,
            WARN,
            ERROR,
            CRITICAL,
            KERNEL
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

        template<class... Args>
        static void logKernel(const std::string& message, Args&&... args)
        {
            Logger::log(LogLevel::KERNEL, std::vformat(message, std::make_format_args(args...)));
        }
    };
};
