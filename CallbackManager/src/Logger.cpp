#include <utility>

#include "../include/Logger.h"

Logger::Logger() = default;

Logger::Logger(const std::string& filename, std::string module, const bool erease)
{
	m_module = std::move(module);
	if (erease)
		m_logfile = new std::ofstream(filename, std::ios::out);
	else
		m_logfile = new std::ofstream(filename, std::ios::app);
	if (m_logfile->fail())
	{
		m_logfile = nullptr;

		std::stringstream debug;
		debug << "[XPLMServer][Logger]Unable to open file : '" << filename << "' (as erease = '" << erease << "')\n";
		Log(debug.str());
	}
}

Logger::~Logger() = default;

std::string Logger::GetModuleName()
{
	return m_module;
}

void Logger::SetModuleName(const std::string& module)
{
	m_module = module;
}

void Logger::Log(std::string const& message, const Logger::Severity severity) const
{
	std::stringstream ss;
	ss << CurrentDateTime() << "\t" << m_module << "\t" << GetSeverityStr(severity)  << "\t" << message << "\n";
#ifdef IBM
	OutputDebugStringA(ss.str().c_str());
#else
	std::clog << ss.str();
#endif
	if (m_logfile == nullptr || m_logfile->fail())
	{
		XPLMDebugString(ss.str().c_str());
		return;
	}
	*m_logfile << ss.str();
	m_logfile->flush();
}

const char* Logger::CurrentDateTime() const
{
	struct tm* ltm;
	time_t now = time(0);
	#ifdef IBM
		ltm = new struct tm;
		localtime_s(ltm, &now);
	#else
		ltm = localtime(&now);
	#endif
	auto time = new char[20];
	#ifdef IBM
		sprintf_s(time, 20,"%02d/%02d/%04d %02d:%02d:%02d", ltm->tm_mday, ltm->tm_mon, ltm->tm_year,
				  ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	#else
		sprintf(time,"%02d/%02d/%04d %02d:%02d:%02d", ltm->tm_mday, ltm->tm_mon, ltm->tm_year,
			    ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	#endif
	return (const char*)time;
}

void Logger::operator+=(const std::string& message) const
{
	Log(message);
}

std::string Logger::GetSeverityStr(Logger::Severity severity)
{
	switch (severity)
	{
	case Logger::Severity::TRACE:
		return "TRACE";
	case Logger::Severity::DEBUG:
		return "DEBUG";
	case Logger::Severity::WARNING:
		return "WARNING";
	case Logger::Severity::CRITICAL:
		return "CRITICAL";
	case Logger::Severity::FATAL:
		return "FATAL";
	default:
		return "";
	}
}
