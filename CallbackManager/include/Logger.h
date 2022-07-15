#pragma once

#ifndef IBM
#include <iostream>
#include <ctime>
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <XPLMUtilities.h>

class Logger final
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
	explicit Logger(const std::string& filename, std::string module="", bool erease = false);
	~Logger();
	std::string GetModuleName();
	void SetModuleName(const std::string& module);
	virtual void Log(std::string const& message, const Logger::Severity severity = Logger::Severity::DEBUG) const;
	const char* CurrentDateTime() const;
	void operator+=(const std::string& message) const;
protected:
static std::string GetSeverityStr(Logger::Severity severity);
	std::string m_module;
	std::ofstream* m_logfile = nullptr;
};