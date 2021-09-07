#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>


class Logger
{
public:
	Logger();
	Logger(std::string filename, bool erease = false);
	//Logger(std::ofstream* logfile, bool erease = false);
	//Logger(const Logger &cpy);
	~Logger();
	virtual void Log(std::string message);
	const char* CurrentDateTime();
	void operator+=(const std::string& message);
protected:
	std::ofstream* m_logfile;
};

