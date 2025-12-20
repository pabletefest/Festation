#include "logger.hpp"

#include <mutex>
#include <print>

static std::mutex s_loggerMutex;

enum class OutputColor {
	DEFAULT,
	YELLOW,
	RED
};

static void printColoredLine(OutputColor color, std::string& message) {
	switch (color) {
	case OutputColor::DEFAULT:
		std::println("{}", message);
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

void Logger::log(Logger::LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lk(s_loggerMutex);

	/*if (static_cast<uint32_t>(level) < static_cast<uint32_t>(s_logLevel))
		return;*/

	switch (level)
	{
	case LogLevel::DEBUG:
		printColoredLine(OutputColor::DEFAULT, std::string("[DEBUG] - ").append(message));
		break;
	case LogLevel::INFO:
		printColoredLine(OutputColor::DEFAULT, std::string("[INFO] - ").append(message));
		break;
	case LogLevel::WARN:
		printColoredLine(OutputColor::YELLOW, std::string("[WARN] - ").append(message));
		break;
	case LogLevel::ERROR:
		printColoredLine(OutputColor::RED, std::string("[ERROR] - ").append(message));
		break;
	case LogLevel::CRITICAL:
		printColoredLine(OutputColor::RED, std::string("[CRITICAL] - ").append(message));
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
