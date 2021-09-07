#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>


class Logger
{
public:
	enum class Severity {
		TRACE,
		DEBUG,
		WARNING,
		CRITICAL,
		FATAL
	};

	Logger();
	Logger(std::string filename, std::string module="", bool erease = false);
	~Logger();
	std::string GetModuleName();
	void SetModuleName(std::string module);
	virtual void Log(std::string message, Logger::Severity severity= Logger::Severity::DEBUG);
	const char* CurrentDateTime();
	void operator+=(const std::string& message);
protected:
	std::string getSeverityStr(Logger::Severity severity);
	std::string m_module;
	std::ofstream* m_logfile;
};

