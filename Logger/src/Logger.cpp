#include "../include/Logger.h"

Logger::Logger() : m_logfile(nullptr), m_module("")
{
}

Logger::Logger(std::string filename, std::string module, bool erease)
{
	m_module = module;
	if (erease)
		m_logfile = new std::ofstream(filename, std::ios::out);
	else
		m_logfile = new std::ofstream(filename, std::ios::app);
	if (m_logfile->fail())
	{
		std::cout << "Unable to open file : '" << filename << "' (as erease = '" << erease << "')\n";
	}
}

//Logger::Logger(std::ofstream* logfile, bool erease)
//{
//}
//
//Logger::Logger(const Logger& cpy)
//{
//}

Logger::~Logger()
{
}

std::string Logger::GetModuleName()
{
	return m_module;
}

void Logger::SetModuleName(std::string module)
{
	m_module = module;
}

void Logger::Log(std::string message, Logger::Severity severity)
{
	if (m_logfile == nullptr || m_logfile->fail())
		return;
	*m_logfile << CurrentDateTime() << "\t" << m_module << "\t" << this->getSeverityStr(severity) \
		<< "\t" << message << "\n";
}

const char* Logger::CurrentDateTime()
{
	time_t now = time(0);
	struct tm* ltm = new tm();
	//struct tm buf;
	localtime_s(ltm, &now);
	char* time = new char[20];
	sprintf_s(time, 20,"%02d/%02d/%04d %02d:%02d:%02d", ltm->tm_mday, ltm->tm_mon, ltm->tm_year,
		ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	std::cout << "Date : '" << time <<"'\n";
	delete ltm;
	return (const char*)time;
}

void Logger::operator+=(const std::string& message)
{
	Log(message);
}

std::string Logger::getSeverityStr(Logger::Severity severity)
{
	switch (severity)
	{
	case Logger::Severity::TRACE:
		return "TRACE";
	case Logger::Severity::DEBUG:
		return "DEBUG";
	case Logger::Severity::WARNING:
		return "WARINING";
	case Logger::Severity::CRITICAL:
		return "CRITICAL";
	case Logger::Severity::FATAL:
		return "FATAL";
	default:
		return "";
	}
}
