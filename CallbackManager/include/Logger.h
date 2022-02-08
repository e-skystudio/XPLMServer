#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <XPLMUtilities.h>

class Logger
{
/*
* Class used to manage logger object.
* Logger output a "csv" style log (data separated by a tabulation")
*/
public:
	enum class Severity {
		TRACE, // Basic information
		DEBUG, // Debug information
		WARNING, // Cautionous information
		CRITICAL, // Information are critical but program can keep running
		FATAL // Crashed
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