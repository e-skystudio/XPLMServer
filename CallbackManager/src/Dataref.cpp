#include "../include/Dataref.h"

/* \brief Default Constructor */
Dataref::Dataref() : 
	m_dataref(nullptr), m_type(Dataref::Type::Unknown),
	m_logger(Logger("XPLMServer.log", "Dataref", false))
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
		value = std::to_string(XPLMGetDatai(m_dataref));
		break;
	case Dataref::Type::Float:
		value = std::to_string(XPLMGetDataf(m_dataref));
		break;
	case Dataref::Type::Double:
		value = std::to_string(XPLMGetDatad(m_dataref));
		break;
	case Dataref::Type::FloatArray:
		break;
	case Dataref::Type::IntArray:
		break;
	case Dataref::Type::Data:
		break;
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
		int val = std::stoi(value);
		XPLMSetDatai(m_dataref, val);
		break;
	}
	case Dataref::Type::Float:
	{
		float val = std::stof(value);
		XPLMSetDataf(m_dataref, val);
		break;
	}
	case Dataref::Type::Double:
	{
		double val = std::stod(value);
		XPLMSetDatad(m_dataref, val);
		break;
	}
	case Dataref::Type::FloatArray:
		break;
	case Dataref::Type::IntArray:
		break;
	case Dataref::Type::Data:
		break;
	default:
		break;
	}
}


