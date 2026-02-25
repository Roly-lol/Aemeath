#include "Logger.h"
#include <cstdarg>
#include <filesystem>
#include <iostream>
#include <windows.h>
Logger::Level Logger::s_ConsoleLogLevel = Logger::Level::None;
std::string Logger::directory = "";
std::string Logger::logfilepath = "";
std::mutex Logger::_mutex{};
void Logger::SetLevel(Level level, LoggerType type)
{
	switch (type)
	{
	case Logger::LoggerType::ConsoleLogger:
		s_ConsoleLogLevel = level;
		break;
	default:
		break;
	}
}
Logger::Level Logger::GetLevel(Logger::LoggerType type)
{
	switch (type)
	{
	case Logger::LoggerType::ConsoleLogger:
		return s_ConsoleLogLevel;
	default:
		return Logger::Level::None;
	}
}
struct Prefix 
{
	char color;
	const char* text;
};
Prefix GetLevelPrefix(Logger::Level level) 
{
	switch (level)
	{
	case Logger::Level::Critical:
		return { 0x04, "Critical" };
	case Logger::Level::Error:
		return { 0x0C, "Error" };
	case Logger::Level::Warning:
		return { 0x06, "Warning" };
	case Logger::Level::Info:
		return { 0x02, "Info" };
	case Logger::Level::Debug:
		return { 0x0B, "Debug" };
	case Logger::Level::Trace:
		return { 0x08, "Trace" };
	default:
		return { 0x00, "" };
	}
} 
#define EXAMPLE_MACRO_NAME
#pragma warning(disable : 4996)
char* GetTime()
{
	static char retime[9];
	time_t current_time; 
	struct tm* time_info;
	time(&current_time); 
	time_info = localtime(&current_time); 
	strftime(retime, 9, "%H:%M:%S", time_info); 
	return retime;
}
void Logger::Log(Logger::Level logLevel, const char* filepath, int line, const char* fmt, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	LogEvent(logLevel,line, buffer);
	if (Logger::s_ConsoleLogLevel == Logger::Level::None )
		return;

	auto filename = std::filesystem::path(filepath).filename().string();

	auto prefix = GetLevelPrefix(logLevel);

	if (Logger::s_ConsoleLogLevel != Logger::Level::None && Logger::s_ConsoleLogLevel >= logLevel)
	{

		const std::lock_guard<std::mutex> lock(_mutex);
		//setlocale(LC_ALL, "zh_CN.UTF-8");//将控制台编码改成utf-8
		//auto logLineConsole = string_format(buffer);
		auto logLineConsole = string_format("[%s:%d]%s", filename.c_str(), line, buffer);//For writeFileName in console 
		std::cout << "["<< GetTime()<< "]";
		//std::cout << "[" << print_trace() << "]";
		std::cout << "[" ;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, prefix.color);
		std::cout << prefix.text;
		SetConsoleTextAttribute(hConsole, 15);
		std::cout << "]" << logLineConsole << std::endl;
		//setlocale(LC_ALL, "C");//将控制台编码恢复ASCII
	}
}

