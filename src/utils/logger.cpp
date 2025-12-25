#include "logger.hpp"

#include <mutex>
#include <print>

static std::mutex s_loggerMutex;

enum class OutputColor {
	DEFAULT,
	LIGHT_BLUE,
	YELLOW,
	RED
};

static void printColoredLine(OutputColor color, std::string& message) {
	switch (color) {
	case OutputColor::DEFAULT:
		std::println("{}", message);
		break;
	case OutputColor::LIGHT_BLUE:
		std::println("\033[96m{}\033[0m", message);
		break;
	case OutputColor::YELLOW:
		std::println("\033[33m{}\033[0m", message);
		break;
	case OutputColor::RED:
		std::println("\033[31m{}\033[0m", message);
		break;
	default:
		std::println("{}", message);
		break;
	}
}

//static Logger::LogLevel s_logLevel = Logger::LogLevel::DEBUG;

void festation::Logger::log(festation::Logger::LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lk(s_loggerMutex);

	/*if (static_cast<uint32_t>(level) < static_cast<uint32_t>(s_logLevel))
		return;*/

	switch (level)
	{
	case LogLevel::DEBUG:
		{
			std::string res = std::format("{:8} - {}", "[DEBUG]", message);
			printColoredLine(OutputColor::DEFAULT, res);
		}
		break;
	case LogLevel::INFO:
		{
			std::string res = std::format("{:8} - {}", "[INFO]", message);
			printColoredLine(OutputColor::DEFAULT, res);
		}
		break;
	case LogLevel::WARN:
		{
			std::string res = std::format("{:8} - {}", "[WARN]", message);
			printColoredLine(OutputColor::YELLOW, res);
		}
		break;
	case LogLevel::ERROR:
		{
			std::string res = std::format("{:8} - {}", "[ERROR]", message);
			printColoredLine(OutputColor::RED, res);
		}		
		break;
	case LogLevel::CRITICAL:
		{
			std::string res = std::format("{:8} - {}", "[CRITICAL]", message);
			printColoredLine(OutputColor::RED, res);
		}		
		break;
	case LogLevel::KERNEL:
		{
			std::string res = std::format("{:8} - {}", "[KERNEL]", message);
			printColoredLine(OutputColor::LIGHT_BLUE, res);
		}		
		break;
	default:
		std::println("UNSUPPORTED LOG LEVEL!");
		break;
	}
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
