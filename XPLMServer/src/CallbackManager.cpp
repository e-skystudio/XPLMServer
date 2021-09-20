#include "CallbackManager.h"

CallbackManager::CallbackManager()
{
	m_callbacks = new std::map<std::string, std::function<int(json, CallbackManager*)>*>();
	m_namedDatarefs = new std::map<std::string, Dataref*>();
	m_subscribedDatarefs = new std::map<std::string, Dataref*>();
}

CallbackManager::CallbackManager(const CallbackManager& rhs)
{
	m_callbacks = rhs.m_callbacks;
	m_namedDatarefs = rhs.m_namedDatarefs;
	m_subscribedDatarefs = rhs.m_subscribedDatarefs;
}

int CallbackManager::AppendCallback(std::string name, std::function<int(json, CallbackManager*)>* newCallback)
{
	if (m_callbacks->contains(name))
	{
		return EXIT_FAILURE;
	}
	m_callbacks->emplace(name, newCallback);
	return EXIT_SUCCESS;
}

std::map<std::string, Dataref*>* CallbackManager::GetNamedDataref() const
{
	return m_namedDatarefs;
}

std::map<std::string, Dataref*>* CallbackManager::GetSubscribedDataref() const
{
	return m_subscribedDatarefs;
}

int CallbackManager::ExecuteCallback(std::string operation, json jsonData)
{
	if (!m_callbacks->contains(operation))
		return EXIT_FAILURE;
	m_callbacks->at(operation)->operator()(jsonData, this);
	return 0;
}
