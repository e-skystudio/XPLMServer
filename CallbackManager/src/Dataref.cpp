#include "../include/Dataref.h"

/* \brief Default Constructor */
Dataref::Dataref() :
	m_dataref(nullptr), m_type(Dataref::Type::Unknown),
	m_logger(Logger("XPLMServer.log", "Dataref", false)),
	m_link(""), m_conversionFactor("1.0")
{
}

/* \brief Copy Constructor */
Dataref::Dataref(const Dataref& rhs)
{

	m_dataref = rhs.m_dataref;
	m_type = rhs.m_type;
}

Dataref::~Dataref()
{
}

/* \brief Load a dataref with a specific path
* \param[in] path the path of the dataref
* \return boolean True if the path return a valid dataref
*/
bool Dataref::Load(std::string path)
{
	m_link = path;
	m_dataref = XPLMFindDataRef(path.c_str());
	if (m_dataref == nullptr)
	{
		m_logger.Log("Loading Dataref '" + path + "' : FAILED", Logger::Severity::WARNING);
		return false;
	}
	m_logger.Log("Loading Dataref '" + path + "' : SUCESS");
	m_type = this->LoadType();
	return true;
}

bool Dataref::CanWrite()
{
	return m_dataref && XPLMCanWriteDataRef(m_dataref);
}

bool Dataref::IsGood()
{
	return m_dataref && XPLMIsDataRefGood(m_dataref);
}

Dataref::Type Dataref::GetType()
{
	return m_type;
}

Dataref::Type Dataref::LoadType()
{
	if (m_dataref != nullptr)
	{
		m_type = (Dataref::Type)XPLMGetDataRefTypes(m_dataref);
		return m_type;
	}
	return Dataref::Type::Unknown;
}

void Dataref::SetType(Dataref::Type newType)
{
	m_type = newType;
}

void Dataref::SetType(std::string newType)
{
	if (StringToType.contains(newType))
	{
		m_type = StringToType.at(newType);
	}
	else {
		m_type = Dataref::Type::Unknown;
	}
}

std::string Dataref::GetValue()
{
	if (m_dataref == NULL)
	{
		return std::string();
	}

	std::string value;
	switch (m_type)
	{
	case Dataref::Type::Unknown:
		break;
	case Dataref::Type::Int:
		value = std::to_string(XPLMGetDatai(m_dataref) * (int)std::stod(m_conversionFactor));
		break;
	case Dataref::Type::Float:
		value = std::to_string(XPLMGetDataf(m_dataref) * (float)std::stod(m_conversionFactor));
		break;
	case Dataref::Type::Double:
		value = std::to_string(XPLMGetDatad(m_dataref) * std::stod(m_conversionFactor));
		break;
	case Dataref::Type::FloatArray:
	{
		int arraySize = XPLMGetDatavf(m_dataref, nullptr, 0, 0);
		XPLMSpeakString(("Array is " + std::to_string(arraySize) + " lenght").c_str());
		float* floatArray = (float*)malloc(sizeof(float) * arraySize);
		XPLMGetDatavf(m_dataref, floatArray, 0, arraySize);
		json j = json::array();
		for (int i = 0; i < arraySize; i++)
		{
			j.push_back((*(floatArray + i)) * (float)std::stod(m_conversionFactor));
		}
		value = j.dump();
		break;
	}
	case Dataref::Type::IntArray:
	{
		int arraySize = XPLMGetDatavi(m_dataref, nullptr, 0, 0);
		XPLMSpeakString(("Array is " + std::to_string(arraySize) + " lenght").c_str());
		int* intArray = (int*)malloc(sizeof(int) * arraySize);
		XPLMGetDatavi(m_dataref, intArray, 0, arraySize);
		json j = json::array();
		for (int i = 0; i < arraySize; i++)
		{
			//j.push_back(*(intArray + i));
			j.push_back((*(intArray + i)) * (int)std::stod(m_conversionFactor));
		}
		value = j.dump();
		break;
	}
	case Dataref::Type::Data:
		break;
	default:
		break;
	}
	m_logger.Log("Link : " + m_link + " = '" + value + "'!");
	return value;
}

void Dataref::SetValue(std::string value)
{
	m_logger.Log("SetValue called!", Logger::Severity::TRACE);
	if (m_dataref == NULL || !this->CanWrite())
	{
		m_logger.Log("Dataref is null or readonly", Logger::Severity::DEBUG);
		return;
	}
	switch (m_type)
	{
	case Dataref::Type::Unknown:
		break;
	case Dataref::Type::Int:
	{
		int val = std::stoi(value) / (int)std::stod(m_conversionFactor);
		XPLMSetDatai(m_dataref, val);
		break;
	}
	case Dataref::Type::Float:
	{
		float val = std::stof(value) / (float)std::stod(m_conversionFactor);
		XPLMSetDatai(m_dataref, val);
		XPLMSetDataf(m_dataref, val);
		break;
	}
	case Dataref::Type::Double:
	{
		double val = std::stod(value) / std::stod(m_conversionFactor);
		XPLMSetDatad(m_dataref, val);
		break;
	}
	case Dataref::Type::FloatArray:
	{
		m_logger.Log("Calling 'setFloatArrayFromJson'", Logger::Severity::TRACE);
		int res = this->setFloatArrayFromJson(0, value);
		m_logger.Log("SetFloatArrayFromJson returned " + std::to_string(res) + "\n");
		break;
	}
	case Dataref::Type::IntArray:
	{
		int res = this->setIntArrayFromJson(0, value);
		m_logger.Log("SetIntArrayFromJson returned " + std::to_string(res) + "\n");
		break;
	}
	case Dataref::Type::Data:
		break;
	default:
		break;
	}
}

void Dataref::SetConversionFactor(std::string conversionFactor)
{
	m_conversionFactor = conversionFactor;
}

int Dataref::setFloatArrayFromJson(int offset, std::string value)
{	
	std::vector<float> data;
	int maxSize = XPLMGetDatavf(m_dataref, nullptr, 0, 0);
	m_logger.Log("Max Size is '" + std::to_string(maxSize) + " '", Logger::Severity::TRACE);
	int f_offset(offset);
	json j = json::parse(value, nullptr, false, false);
	m_logger.Log("DataType is : '" + std::to_string((int)j.type()) + "'", Logger::Severity::DEBUG);
	if (j.type() == json::value_t::discarded)
	{
		m_logger.Log("FloatArray : json parsing of value failed!", Logger::Severity::CRITICAL);
		return -1;
	}
	else if (j.type() == json::value_t::array)
	{
		m_logger.Log("FloatArray : value is an array!", Logger::Severity::DEBUG);
		if ((int)j.size() < maxSize)
			maxSize = (int)j.size();
		m_logger.Log("FloatArray : max size is " + std::to_string(maxSize), Logger::Severity::CRITICAL);
		std::vector<std::string> valArray = j.get<std::vector<std::string>>();
		for (std::string strValue : valArray)
		{
			data.push_back(std::stof(strValue));
		}
	}
	else if (j.type() == json::value_t::string || j.type() == json::value_t::number_float)
	{
		m_logger.Log("FloatArray : value is an string!", Logger::Severity::DEBUG);
		if (j.type() == json::value_t::number_float)
		{
			m_logger.Log("Value is '" + std::to_string(j.get<float>()) + "'[FLOAT]", Logger::Severity::DEBUG);
			for (int i(0); i < maxSize; i++)
			{
				data.push_back(j.get<float>() / (float)std::stod(m_conversionFactor));
			}
		}
		else {
			m_logger.Log("Value is '" + j.get<std::string>() + "'[STR]", Logger::Severity::DEBUG);

		}
	}
	else if (j.type() == json::value_t::object)
	{
		m_logger.Log("FloatArray : value is an json object!", Logger::Severity::DEBUG);
		if (!j.contains("Value"))
		{
			m_logger.Log("FloatArray : json is not an array and doesn't contain a Value field", Logger::Severity::CRITICAL);
			return -2;
		}
		if (j.contains("Offset"))
		{
			if (j["Offset"].type() == json::value_t::string)
			{
				f_offset = std::stoi(j["Offset"].get<std::string>()) / (int)std::stod(m_conversionFactor);
			}
			else
			{
				m_logger.Log("FloatArray : Offset field exist but is not string... skipping", Logger::Severity::WARNING);
				f_offset = 0;
			}
		}
		return setFloatArrayFromJson(f_offset, j["Value"].dump());
	}
	else {
		m_logger.Log("FloatArray: JSON type is unknown", Logger::Severity::CRITICAL);
		return 0x02;
	}
	m_logger.Log("FloatArray: Size : " + std::to_string(maxSize) + "Offset : " + std::to_string(offset), Logger::Severity::TRACE);
	data.resize(maxSize);
	XPLMSetDatavf(m_dataref, data.data(), offset, maxSize);
	return EXIT_SUCCESS;
}

int Dataref::setIntArrayFromJson(int offset, std::string value)
{
	std::vector<int> data;
	int maxSize = XPLMGetDatavi(m_dataref, nullptr, 0, 0);
	m_logger.Log("Max Size is '" + std::to_string(maxSize) + " '", Logger::Severity::TRACE);
	int f_offset(offset);
	json j = json::parse(value, nullptr, false, false);
	m_logger.Log("DataType is : '" + std::to_string((int)j.type()) + "'", Logger::Severity::DEBUG);
	if (j.type() == json::value_t::discarded)
	{
		m_logger.Log("IntArray : json parsing of value failed!", Logger::Severity::CRITICAL);
		return -1;
	}
	else if (j.type() == json::value_t::array)
	{
		m_logger.Log("IntArray : value is an array!", Logger::Severity::DEBUG);
		if ((int)j.size() < maxSize)
			maxSize = (int)j.size();
		m_logger.Log("IntArray : max size is " + std::to_string(maxSize), Logger::Severity::CRITICAL);
		std::vector<std::string> valArray = j.get<std::vector<std::string>>();
		for (std::string strValue : valArray)
		{
			data.push_back(std::stoi(strValue));
		}
	}
	else if (j.type() == json::value_t::string || j.type() == json::value_t::number_integer || j.type() == json::value_t::number_unsigned)
	{
		m_logger.Log("IntArray : value is an string!", Logger::Severity::DEBUG);
		if (j.type() == json::value_t::number_integer)
		{
			m_logger.Log("Value is '" + std::to_string(j.get<int>()) + "'[INT]", Logger::Severity::DEBUG);
			for (int i(0); i < maxSize; i++)
			{
				data.push_back(j.get<int>());
			}
		}
		else if (j.type() == json::value_t::number_unsigned)
		{
			m_logger.Log("Value is '" + std::to_string(j.get<unsigned int>()) + "'[UINT]", Logger::Severity::DEBUG);
			for (int i(0); i < maxSize; i++)
			{
				data.push_back(j.get<unsigned int>());
			}
		}
		else {
			std::string valueStr = j.get<std::string>();
			m_logger.Log("Value is '" + valueStr + "'[STR]", Logger::Severity::DEBUG);
			for (int i(0); i < maxSize; i++)
			{
				data.push_back(std::stoi(valueStr));
			}
		}
	}
	else if (j.type() == json::value_t::object)
	{
		m_logger.Log("IntArray : value is an json object!", Logger::Severity::DEBUG);
		if (!j.contains("Value"))
		{
			m_logger.Log("IntArray : json is not an array and doesn't contain a Value field", Logger::Severity::CRITICAL);
			return -2;
		}
		if (j.contains("Offset"))
		{
			if (j["Offset"].type() == json::value_t::string)
			{
				f_offset = std::stoi(j["Offset"].get<std::string>());
			}
			else
			{
				m_logger.Log("IntArray : Offset field exist but is not string... skipping", Logger::Severity::WARNING);
				f_offset = 0;
			}
		}
		return setIntArrayFromJson(f_offset, j["Value"].dump());
	}
	else {
		m_logger.Log("IntArray: JSON type is unknown", Logger::Severity::CRITICAL);
		return 0x02;
	}
	m_logger.Log("IntArray: Size : " + std::to_string(maxSize) + "Offset : " + std::to_string(offset), Logger::Severity::TRACE);
	data.resize(maxSize);
	XPLMSetDatavi(m_dataref, data.data(), offset, maxSize);
	return EXIT_SUCCESS;
}
