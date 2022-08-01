#include "../include/FFDataref.h"

FFDataref::FFDataref() : m_id(-1),
	m_type(Type::Deleted), m_link(""),
	m_conversionFactor("1.0"),
	m_ffapi(nullptr),
	m_logger(Logger("XPLMServer.log", "FFDataref", false)),
	m_needUpdate(false)
{
	DatarefType = DatarefType::FFDataref;
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
		if (lenght <= 0) return "";
		void* buffer = malloc(sizeof(char));
		m_ffapi->ValueGet(m_id, buffer);
		auto str = (char*)malloc( lenght + 1);
		memset(str, 0x00, lenght + 1);
		memcpy_s(str, lenght, buffer, lenght);
		m_logger.Log(str);
		std::string value(str);
		return value;
	}
	default:
		m_logger.Log(m_link + ": has type " + std::to_string((int)m_type));
		return "N/A";
	}
}

void FFDataref::SetValue(std::string value)
{
	m_targetValue = value;
	m_needUpdate = true;
}

void FFDataref::DoSetValue(std::string value)
{
	m_logger.Log("[DO_SET_VALUE]" + m_link + " Setting value to " + value);
	int type = m_ffapi->ValueType(m_id);
	if (type == Value_Type_float32)
	{
		m_logger.Log("[DO_SET_VALUE]" + m_link + " Type is  " + std::to_string(type) + " (float32)");
		float val(0.0f);
		try
		{
			val = std::stof(value);
		}
		catch (std::invalid_argument)
		{
			m_logger.Log("[DO_SET_VALUE]" + m_link + " std::stof has crashed because of an invalid argument", Logger::Severity::CRITICAL);
			return;
		}
		m_ffapi->ValueSet(m_id, &val);
	}
	else if (type == Value_Type_float64)
	{
		m_logger.Log("[DO_SET_VALUE]" + m_link + " Type is  " + std::to_string(type) + " (float64)");
		double val(0.0);
		try
		{
			val = std::stod(value);
		}
		catch (std::invalid_argument)
		{
			m_logger.Log("[DO_SET_VALUE]" + m_link + " std::stof has crashed because of an invalid argument", Logger::Severity::CRITICAL);
			return;
		}
		m_ffapi->ValueSet(m_id, &val);
	}
	else if (type == Value_Type_sint32)
	{
		m_logger.Log("[DO_SET_VALUE]" + m_link + " Type is  " + std::to_string(type) + " (sint32)");
		int val(0);
		try
		{
			val = std::stoi(value);
		}
		catch (std::invalid_argument)
		{
			m_logger.Log("[DO_SET_VALUE]" + m_link + " std::stof has crashed because of an invalid argument", Logger::Severity::CRITICAL);
			return;
		}
		m_ffapi->ValueSet(m_id, &val);
	}
}

std::string FFDataref::GetTargetValue()
{
	return m_targetValue;
}
#pragma region disabled
//void FFDataref::SetTargetValue()
//{
//	m_needUpdate = false;
//	XPLMSpeakString(std::string("Updating value for '" + m_link + "'!").c_str());
//	if(m_ffapi == nullptr) {
//		m_logger.Log(m_link + ": FlightFactor API pointer is NULL!", Logger::Severity::WARNING);
//		return;
//	};
//	if (m_id < 0 && m_type == Type::Deleted)
//	{
//		if (m_id < 0) m_logger.Log(m_link + ": id is INVALID ID", Logger::Severity::WARNING);
//		if (m_type == Type::Deleted) m_logger.Log(m_link + ": type is 'Deleted'", Logger::Severity::WARNING);
//		return;
//	}
//	double converstionfactor = std::stod(m_conversionFactor);
//	void* val(nullptr);
//	switch (m_type)
//	{
//	case FFDataref::Type::Char:
//	{
//		val = malloc(sizeof(char));
//		*(char*)val = (char)(std::stoi(m_targetValue) * (int)converstionfactor);
//	}
//	case FFDataref::Type::uChar:
//	{
//		val = malloc(sizeof(char));
//		*(unsigned char*)val = (unsigned char)(std::stoi(m_targetValue) * (int)converstionfactor);
//	}
//	case FFDataref::Type::Short:
//	{
//		val = malloc(sizeof(short));
//		*(short*)val = (short)(std::stoi(m_targetValue) * (int)converstionfactor);
//	}
//	case FFDataref::Type::uShort:
//	{
//		val = malloc(sizeof(short));
//		*(unsigned short*)val = (unsigned short)(std::stoi(m_targetValue) * (int)converstionfactor);
//	}
//	case FFDataref::Type::Int:
//	{
//		val = malloc(sizeof(int));
//		*(int*)val = (int)(std::stoi(m_targetValue) * (int)converstionfactor);
//	}
//	case FFDataref::Type::uInt:
//	{
//		val = malloc(sizeof(int));
//		*(unsigned int*)val = (unsigned int)(std::stoi(m_targetValue) * (unsigned int)converstionfactor);
//		m_ffapi->ValueSet(m_id, val);
//	}
//	case FFDataref::Type::Float:
//	{
//		val = malloc(sizeof(float));
//		*(float*)val = (float)(std::stod(m_targetValue) * (double)converstionfactor);
//	}
//	case FFDataref::Type::Double:
//	{
//		val = malloc(sizeof(double));
//		*(double*)val = (std::stod(m_targetValue) * converstionfactor);
//	}
//	case FFDataref::Type::String:
//	{
//		int lenght = m_ffapi->ValueGetSize(m_id);
//		char* buffer = new char[lenght];
//		#ifdef IBM
//		strcpy_s(buffer, lenght, m_targetValue.c_str());
//		#else
//		strcpy(buffer, m_targetValue.c_str());
//		#endif
//		m_ffapi->ValueSet(m_id, buffer);
//		delete[] buffer;
//	}
//	default:
//		return;
//	}
//	m_ffapi->ValueSet(m_id, val);
//	m_logger.Log("Reinitalising Dataref");
//	m_targetValue = "";
//	free(val);
//}
#pragma endregion
bool FFDataref::NeedUpdate() const
{
	return m_needUpdate;
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

int FFDataref::GetID() const
{
	return m_id;
}

int FFDataref::GetFlag() const
{
	return m_ffapi->ValueType(m_id);
}

int FFDataref::GetUnit() const
{
	return m_ffapi->ValueUnits(m_id);
}

std::string FFDataref::GetName() const
{
	return std::string(m_ffapi->ValueName(m_id));
}

std::string FFDataref::GetDescription() const
{
	return std::string(m_ffapi->ValueDesc(m_id));
}