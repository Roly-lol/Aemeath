#pragma once
#include <string>
#include <mutex>
#include "event.hpp"

#define EXTLOG(level, fmt, ...) Logger::Log(level, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define LOG_CRIT(fmt, ...) EXTLOG(Logger::Level::Critical, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) EXTLOG(Logger::Level::Error, fmt, __VA_ARGS__)
#define LOG_WARNING(fmt, ...) EXTLOG(Logger::Level::Warning, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) EXTLOG(Logger::Level::Info, fmt, __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) EXTLOG(Logger::Level::Debug, fmt, __VA_ARGS__)
#define LOG_TRACE(fmt, ...) EXTLOG(Logger::Level::Trace, fmt, __VA_ARGS__)

class Logger 
{
public:
	enum class Level 
	{
		None,
		Critical,
		Error,
		Warning,
		Info,
		Debug,
		Trace
	};

	enum class LoggerType 
	{
		ConsoleLogger,
	};

	static void SetLevel(Level level, LoggerType type = LoggerType::ConsoleLogger);
	static Level GetLevel(LoggerType type);

	static void Log(Level logLevel, const char* filename, int line, const char* fmt, ...);

	//static void PrepareFileLogging(std::string directory);

	static inline TEvent<Logger::Level, int, const char*> LogEvent;

private:
	static Level s_ConsoleLogLevel;
	struct Prefix;
	static std::mutex _mutex;

	static std::string directory;
	static std::string logfilepath;

	template<typename ... Args>
	static std::string string_format(const std::string& format, Args ... args)
	{
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
		auto size = static_cast<size_t>(size_s);
		auto buf = std::make_unique<char[]>(size);
		std::snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

};
