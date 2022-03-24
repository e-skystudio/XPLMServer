#include "../include/Dataref.h"

/* \brief Default Constructor */
Dataref::Dataref() :
	m_dataref(nullptr), m_type(Dataref::Type::Unknown),
	m_logger(Logger("XPLMServer.log", "Dataref", false)),
	m_link(""), 
	m_conversionFactor("1.0")
{
	DatarefType = DatarefType::XPLMDataref;
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
		// handling case where data can be either double or int
		//if ((int)m_type && ((int)Type::Float || (int)Type::Double))
		if ((int)m_type & ((int)Type::Float | (int)Type::Double))
		{
			m_type = Type::Double;
		}
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
		float* floatArray = (float*)malloc(sizeof(float) * arraySize);
		if (floatArray == nullptr) return "";
		XPLMGetDatavf(m_dataref, floatArray, 0, arraySize);
		json j = json::array();
		for (int i = 0; i < arraySize; i++)
		{
			float item = *((float*)(floatArray + i));
			j.push_back(item * (float)std::stod(m_conversionFactor));
		}
		value = j.dump();
		free(floatArray);
		break;
	}
	case Dataref::Type::IntArray:
	{
		int arraySize = XPLMGetDatavi(m_dataref, nullptr, 0, 0);
		int* intArray = (int*)malloc(sizeof(int) * arraySize);
		if (intArray == nullptr) return "";
		XPLMGetDatavi(m_dataref, intArray, 0, arraySize);
		json j = json::array();
		for (int i = 0; i < arraySize; i++)
		{
			int item = *((int*)(intArray + i));
			j.push_back(item * (int)std::stod(m_conversionFactor));
		}
		value = j.dump();
		free(intArray);
		break;
	}
	case Dataref::Type::Data:
	{
		int lenght = XPLMGetDatab(m_dataref, NULL, 0, 0);
		char* data = (char*)malloc(lenght * sizeof(char));
		if (data == nullptr) return "";
		memset(data, 0x00, lenght);
		XPLMGetDatab(m_dataref, (void*)data, 0, lenght);
		value = std::string(data).substr(0, strlen(data));
		m_logger.Log("[GET DATA]" + m_link + " = " + data);
		free(data);
		break;
	}
	default:
		break;
	}
	return value;
}

void Dataref::SetValue(std::string value)
{
	if (m_dataref == NULL || !this->CanWrite())
	{
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
		int res = this->setFloatArrayFromJson(0, value);
		break;
	}
	case Dataref::Type::IntArray:
	{
		int res = this->setIntArrayFromJson(0, value);
		break;
	}
	case Dataref::Type::Data:
	{
		int maxLenght = XPLMGetDatab(m_dataref, NULL, 0, 0);
		char* zero = (char*)malloc(sizeof(char) * maxLenght);
		if (zero == nullptr) return;
		memset(zero, 0x00, maxLenght);
		XPLMSetDatab(m_dataref, zero, 0, maxLenght);
		free(zero);

		m_logger.Log("[SET DATA]" + m_link + " has a max size of" + std::to_string(maxLenght));
		std::size_t lenght = value.size();
		m_logger.Log("[SET DATA]" + value + " has a size of" + std::to_string(lenght));
		if (lenght > maxLenght)
		{
			lenght = maxLenght;
		}
		m_logger.Log("[SET DATA]" + m_link + " size of" + std::to_string(lenght));
		XPLMSetDatab(m_dataref, (void*)value.c_str(), 0, (int)lenght);
		m_logger.Log("[SET DATA]" + m_link + " = " + value);
		break;
	}
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
	int f_offset(offset);
	json j = json::parse(value, nullptr, false, false);
	if (j.type() == json::value_t::discarded)
	{
		m_logger.Log("FloatArray : json parsing of value failed!", Logger::Severity::CRITICAL);
		return -1;
	}
	else if (j.type() == json::value_t::array)
	{
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
		if (j.type() == json::value_t::number_float)
		{
			for (int i(0); i < maxSize; i++)
			{
				data.push_back(j.get<float>() / (float)std::stod(m_conversionFactor));
			}
		}
	}
	else if (j.type() == json::value_t::object)
	{
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
	data.resize(maxSize);
	XPLMSetDatavf(m_dataref, data.data(), offset, maxSize);
	return EXIT_SUCCESS;
}

int Dataref::setIntArrayFromJson(int offset, std::string value)
{
	std::vector<int> data;
	int maxSize = XPLMGetDatavi(m_dataref, nullptr, 0, 0);
	int f_offset(offset);
	json j = json::parse(value, nullptr, false, false);
	if (j.type() == json::value_t::discarded)
	{
		m_logger.Log("IntArray : json parsing of value failed!", Logger::Severity::CRITICAL);
		return -1;
	}
	else if (j.type() == json::value_t::array)
	{
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
		if (j.type() == json::value_t::number_integer)
		{
			for (int i(0); i < maxSize; i++)
			{
				data.push_back(j.get<int>());
			}
		}
		else if (j.type() == json::value_t::number_unsigned)
		{
			for (int i(0); i < maxSize; i++)
			{
				data.push_back(j.get<unsigned int>());
			}
		}
		else {
			std::string valueStr = j.get<std::string>();
			for (int i(0); i < maxSize; i++)
			{
				data.push_back(std::stoi(valueStr));
			}
		}
	}
	else if (j.type() == json::value_t::object)
	{
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
	data.resize(maxSize);
	XPLMSetDatavi(m_dataref, data.data(), offset, maxSize);
	return EXIT_SUCCESS;
}
