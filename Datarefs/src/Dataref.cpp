#include "../include/Dataref.h"

Dataref::Dataref() : 
	m_dataref(nullptr), m_type(Dataref::Type::Unknown),
	m_logger(Logger("XPLMServer.log", "Dataref", false))
{
}

Dataref::Dataref(const Dataref& rhs)
{
	m_dataref = rhs.m_dataref;
	m_type = rhs.m_type;
}

bool Dataref::Load(std::string path)
{
	m_dataref = XPLMFindDataRef(path.c_str());
	if (m_dataref != nullptr)
		m_logger.Log("Loading Dataref '" + path + "' : SUCESS");
	else
		m_logger.Log("Loading Dataref '" + path + "' : SUCESS", Logger::Severity::WARNING);

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


