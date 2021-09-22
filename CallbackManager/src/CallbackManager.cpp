#include "../include/CallbackManager.h"

CallbackManager::CallbackManager() : m_logger(Logger("XPLMServer.log", "CallbackManager", false))
{
	m_callbacks = new std::map<std::string, callback>();
	m_namedDatarefs = new std::map<std::string, Dataref*>();
	m_subscribedDatarefs = new std::map<std::string, Dataref*>();
}

CallbackManager::CallbackManager(const CallbackManager& rhs)
{
	m_callbacks = rhs.m_callbacks;
	m_namedDatarefs = rhs.m_namedDatarefs;
	m_subscribedDatarefs = rhs.m_subscribedDatarefs;
	m_logger = rhs.m_logger;
}

int CallbackManager::AppendCallback(std::string name, callback newCallback)
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

int CallbackManager::LoadCallbackDLL(std::string inDllPath)
{
	HINSTANCE hDLL = LoadLibrary(s2ws(inDllPath).c_str());
	if (hDLL == nullptr)
	{
		std::stringstream ss;
		ss << "DLL : '" << inDllPath << "' WAS NOT FOUND !\n";
		m_logger.Log(ss.str(), Logger::Severity::CRITICAL);
		return -1;
	}
	std::stringstream ss;
	ss << "DLL : '" << inDllPath << "' LOADED SUCESSFULLY !\n";
	m_logger.Log(ss.str(), Logger::Severity::TRACE);

	callbackLoader loader = (callbackLoader)GetProcAddress(hDLL, "GetCallbacks");
	int size = 0;
	loader(nullptr, &size);

	ss = std::stringstream();
	ss << "There is/are " << size << " callback(s) loadable\n";
	m_logger.Log(ss.str(), Logger::Severity::DEBUG);

	CallbackFunction* p_callbacks = new CallbackFunction[size];
	loader(&p_callbacks, &size);
	for (int i(0); i < size; i++)
	{
		CallbackFunction* callback1 = p_callbacks + i;
		m_logger.Log("Trying to load '" + std::string(callback1->function) + "' as '" + 
			std::string(callback1->operation) +"'", Logger::Severity::DEBUG);
		callback p_callback = (callback)GetProcAddress(hDLL, callback1->function);
		int res = this->AppendCallback(std::string(callback1->operation), p_callback);
		if (res != EXIT_SUCCESS)
		{
			m_logger.Log("loading of '" + std::string(callback1->function) + "' as '" + 
				std::string(callback1->operation) + "' has failed!", Logger::Severity::WARNING);
			continue;
		}
#ifdef _DEBUG
		m_logger.Log("loading of '" + std::string(callback1->function) + "' as '" +
			std::string(callback1->operation) + "' sucessfull!", Logger::Severity::DEBUG);
#endif
	}
	return size;
}

int CallbackManager::ExecuteCallback(std::string operation, json* jsonData)
{
	if (!m_callbacks->contains(operation))
		return EXIT_FAILURE;
	//m_callbacks->at(operation)->operator()(jsonData, this);
	m_callbacks->at(operation)(jsonData, this);
	return 0;
}
