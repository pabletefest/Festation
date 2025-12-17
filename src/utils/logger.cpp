#include "logger.hpp"

#include <mutex>
#include <print>

static std::mutex s_loggerMutex;

//static Logger::LogLevel s_logLevel = Logger::LogLevel::DEBUG;

void Logger::log(Logger::LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lk(s_loggerMutex);

	/*if (static_cast<uint32_t>(level) < static_cast<uint32_t>(s_logLevel))
		return;*/

	switch (level)
	{
	default:
		break;
	}

	std::println("{}", message);
}

//Logger& Logger::getLogger()
//{
//	static Logger s_logger;
//	return s_logger;
//}

//void Logger::setLogLevel(LogLevel level)
//{
//	std::lock_guard<std::mutex> lk(s_loggerMutex);
//	s_logLevel = level;
//}
