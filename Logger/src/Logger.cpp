#include "../include/Logger.h"

Logger::Logger() : m_logfile(nullptr)
{
}

Logger::Logger(std::string filename, bool erease)
{
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

void Logger::Log(std::string message)
{
	if (m_logfile == nullptr || m_logfile->fail())
		return;
	*m_logfile << CurrentDateTime() << "\t" << message << "\n";
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
