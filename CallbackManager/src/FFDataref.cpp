#include "../include/FFDataref.h"

FFDataref::FFDataref() : m_id(-1),
	m_type(Type::Deleted), m_link(""),
	m_conversionFactor("1.0"), 
	m_ffapi(nullptr),
	m_logger(Logger("XPLMServer.log", "FFDataref", false))
{
	DatarefType = "FFDataref";
}

FFDataref::FFDataref(SharedValuesInterface* FF_A320_api) : FFDataref()
{
	BindAPI(FF_A320_api);
}

FFDataref::FFDataref(const FFDataref& rhs)
{
	m_id = rhs.m_id;
	m_conversionFactor = rhs.m_conversionFactor;
	m_link = rhs.m_link;
	m_type = rhs.m_type;
}

FFDataref::~FFDataref()
{}

bool FFDataref::Load(std::string path)
{
	m_link = path;
	m_id = m_ffapi->ValueIdByName(path.c_str());
	if(m_id < 0) return false;
	Type t = LoadType();
	return t != Type::Deleted;
}

FFDataref::Type FFDataref::GetType()
{
	return m_type;
}

FFDataref::Type FFDataref::LoadType()
{
	if (m_id < 0) {
		m_logger.Log(m_link + " : Id is less than 1");
		return Type::Deleted;
	}
	m_type = (Type)m_ffapi->ValueType(m_id);
	m_logger.Log(std::string("[" + m_link + "] = " + std::to_string((int)m_type) + "\n").c_str());
	return m_type;
}

std::string FFDataref::GetValue()
{
	m_logger.Log("FFDATAREF GetValue");
	if(m_ffapi == nullptr) {
		m_logger.Log(m_link + ": FlightFactor API pointer is NULL!", Logger::Severity::WARNING);
		return "FFAPI NOT FOUND";
	};
	if (m_id < 0 && m_type == Type::Deleted)
	{
		if (m_id < 0) m_logger.Log(m_link + ": id is INVALID ID", Logger::Severity::WARNING);
		if (m_type == Type::Deleted) m_logger.Log(m_link + ": type is 'Deleted'", Logger::Severity::WARNING);
		return "ID OR TYPE INVALID";
	}	
	m_logger.Log(m_link + " has valid ID/TYPE and FFAPI Is avaialble");
	double converstionfactor = std::stod(m_conversionFactor);
	switch (m_type)
	{
	case FFDataref::Type::Char:
	{
		char data(0);
		m_ffapi->ValueGet(m_id, &data);
		return std::to_string(data * (int)converstionfactor);
	}
	case FFDataref::Type::uChar:
	{
		unsigned char data(0);
		m_ffapi->ValueGet(m_id, &data);
		return std::to_string(data * (int)converstionfactor);
	}
	case FFDataref::Type::Short:
	{
		short data(0);
		m_ffapi->ValueGet(m_id, &data);
		return std::to_string(data * (int)converstionfactor);
	}
	case FFDataref::Type::uShort:
	{
		unsigned short data(0);
		m_ffapi->ValueGet(m_id, &data);
		return std::to_string(data * (int)converstionfactor);
	}
	case FFDataref::Type::Int:
	{
		int data(0);
		m_ffapi->ValueGet(m_id, &data);
		return std::to_string(data * (int)converstionfactor);
	}
	case FFDataref::Type::uInt:
	{
		unsigned int data(0);
		m_ffapi->ValueGet(m_id, &data);
		return std::to_string(data * (int)converstionfactor);
	}
	case FFDataref::Type::Float:
	{
		float data(0.0f);
		m_ffapi->ValueGet(m_id, &data);
		return std::to_string(data * (float)converstionfactor);
	}
	case FFDataref::Type::Double:
	{
		double data(0.0);
		m_ffapi->ValueGet(m_id, &data);
		return std::to_string(data * (double)converstionfactor);
	}
	case FFDataref::Type::String:
	{
		int lenght = m_ffapi->ValueGetSize(m_id);
		char* buffer = new char[lenght];
		m_ffapi->ValueGet(m_id, &buffer);
		std::string value(buffer);
		delete[] buffer;
		return value;
	}
	default:
		m_logger.Log(m_link + ": has type " + std::to_string((int)m_type));
		return "N/A";
	}
}

void FFDataref::SetValue(std::string value)
{
	if(m_ffapi == nullptr) {
		m_logger.Log(m_link + ": FlightFactor API pointer is NULL!", Logger::Severity::WARNING);
		return;
	};
	if (m_id < 0 && m_type == Type::Deleted)
	{
		if (m_id < 0) m_logger.Log(m_link + ": id is INVALID ID", Logger::Severity::WARNING);
		if (m_type == Type::Deleted) m_logger.Log(m_link + ": type is 'Deleted'", Logger::Severity::WARNING);
		return;
	}
	m_logger.Log(m_link + " has valid ID/TYPE and FFAPI Is avaialble");
	double converstionfactor = std::stod(m_conversionFactor);
	switch (m_type)
	{
	case FFDataref::Type::Char:
	{
		char val = (char)(std::stoi(value) * (int)converstionfactor);
		m_ffapi->ValueSet(m_id, &val);
	}
	case FFDataref::Type::uChar:
	{
		unsigned char val = (unsigned char)(std::stoi(value) * (int)converstionfactor);
		m_ffapi->ValueSet(m_id, &val);
	}
	case FFDataref::Type::Short:
	{
		short val = (short)(std::stoi(value) * (int)converstionfactor);
		m_ffapi->ValueSet(m_id, &val);
	}
	case FFDataref::Type::uShort:
	{
		unsigned short val = (unsigned short)(std::stoi(value) * (int)converstionfactor);
		m_ffapi->ValueSet(m_id, &val);
	}
	case FFDataref::Type::Int:
	{
		int val = (int)(std::stoi(value) * (int)converstionfactor);
		m_ffapi->ValueSet(m_id, &val);
	}
	case FFDataref::Type::uInt:
	{
		unsigned int val = (unsigned int)(std::stoi(value) * (int)converstionfactor);
		m_ffapi->ValueSet(m_id, &val);
	}
	case FFDataref::Type::Float:
	{
		float val = (float)(std::stod(value) * (double)converstionfactor);
		m_ffapi->ValueSet(m_id, &val);
	}
	case FFDataref::Type::Double:
	{
		double val = (std::stod(value) * converstionfactor);
		m_ffapi->ValueSet(m_id, &val);
	}
	case FFDataref::Type::String:
	{
		int lenght = m_ffapi->ValueGetSize(m_id);
		char* buffer = new char[lenght];
		#ifdef IBM
		strcpy_s(buffer, lenght, value.c_str());
		#else
		strcpy(buffer, value.c_str());
		#endif
		m_ffapi->ValueSet(m_id, buffer);
		delete[] buffer;
	}
	default:
		return;
	}
}

void FFDataref::BindAPI(SharedValuesInterface* FF_A320_api)
{
	if (FF_A320_api != nullptr)
	{
		m_ffapi = FF_A320_api;
	}
}

void FFDataref::SetConversionFactor(std::string conversionFactor)
{
	m_conversionFactor = conversionFactor;
}
