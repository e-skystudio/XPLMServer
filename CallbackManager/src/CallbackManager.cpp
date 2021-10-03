#include "../include/CallbackManager.h"

CallbackManager::CallbackManager() : m_logger(Logger("XPLMServer.log", "CallbackManager", false))
{
	m_callbacks = new std::map<std::string, callback>();
	m_namedDatarefs = new std::map<std::string, Dataref*>();
	m_subscribedDatarefs = new std::map<std::string, Dataref*>();
	m_subscribedEvent = new std::map<unsigned int, std::string>{
		{101, "Crashed"},
		{102, "Plane Loaded"},
		{103, "Airport Loaded"},
	};
}

CallbackManager::CallbackManager(const CallbackManager& rhs)
{
	m_callbacks = rhs.m_callbacks;
	m_namedDatarefs = rhs.m_namedDatarefs;
	m_subscribedDatarefs = rhs.m_subscribedDatarefs;
	m_logger = rhs.m_logger;
}

CallbackManager::~CallbackManager()
{
	auto kv = m_namedDatarefs->begin();
	while(kv != m_namedDatarefs->end())
	{
		m_namedDatarefs->erase(kv->first);
		if (m_subscribedDatarefs->contains(kv->first))
		{
			m_subscribedDatarefs->erase(kv->first);
		}
		delete kv->second;
		kv++;
	}
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

std::map<unsigned int, std::string>* CallbackManager::GetSubscribedEventMap() const
{
	return m_subscribedEvent;
}

int CallbackManager::LoadCallbackDLL(std::string inDllPath)
{
	HINSTANCE hDLL = LoadLibrary(s2ws(inDllPath).c_str());
	if (hDLL == nullptr)
	{
		std::stringstream ss;
		ss << "DLL : '" << inDllPath << "' WAS NOT FOUND !";
		m_logger.Log(ss.str(), Logger::Severity::CRITICAL);
		m_logger.Log(ss.str().c_str());
		return -1;
	}
	std::stringstream ss;
	ss << "DLL : '" << inDllPath << "' LOADED SUCESSFULLY !";
	m_logger.Log(ss.str(), Logger::Severity::TRACE);
	m_logger.Log(ss.str().c_str());

	callbackLoader loader = (callbackLoader)GetProcAddress(hDLL, "GetCallbacks");
	int size = 0;
	loader(nullptr, &size);

	ss = std::stringstream();
	ss << "There is/are " << size << " callback(s) loadable";
	//m_logger.Log(ss.str(), Logger::Severity::DEBUG);
	m_logger.Log(ss.str().c_str());

	m_logger.Log("Creating an array of CallbackFunction...");
	std::vector<CallbackFunction*> vec_callbacks;
	m_logger.Log("Creating an array of CallbackFunction...[DONE]\n");
	m_logger.Log("Loading the callbacks...");
	loader(&vec_callbacks, &size);
	m_logger.Log("Loading the callbacks...[DONE]\n");
	
	for (std::size_t i(0); i < vec_callbacks.size(); i++)
	{
		m_logger.Log(("Loading callback " + std::to_string(i) + " / " + std::to_string(size - 1) + "\n").c_str());
		CallbackFunction* callback1 = vec_callbacks[i];
		m_logger.Log(("Trying to load '" + callback1->function + "' as '" + callback1->operation + "'...").c_str());
		callback p_callback = (callback)GetProcAddress(hDLL, callback1->function.c_str());
		if (p_callback == nullptr)
		{
			m_logger.Log("pointer callback is pointing to nullptr\n", Logger::Severity::WARNING);
			continue;
		}
		else
		{
			m_logger.Log("pointer callback is valid\n");
		}
		int res = this->AppendCallback(std::string(callback1->operation), p_callback);
		if (res != EXIT_SUCCESS)
		{
			m_logger.Log("Appending Callback to list : [FAILED]\n", Logger::Severity::WARNING);
			continue;
		}
#ifdef _DEBUG
		m_logger.Log("loading of '" + std::string(callback1->function) + "' as '" +
			std::string(callback1->operation) + "' sucessfull!", Logger::Severity::DEBUG);
#endif
	}
	m_logger.Log("LoadCallbackDLL...[FINISHED]\n");
	return size;
}

void CallbackManager::Log(std::string data, Logger::Severity severity)
{
	m_logger.Log(data, severity);
}

int CallbackManager::ExecuteCallback(json* jsonData)
{
	if (!jsonData->contains("Operation"))
		return 0x01;
	
	std::string operation = jsonData->at("Operation").get<std::string>();
	m_logger.Log("Operation '" + operation + "' was requested");
	XPLMDebugString(("Operation '" + operation + "' was requested\n").c_str());

	if (!m_callbacks->contains(operation))
	{
		m_logger.Log("Operation '" + operation + "' was not found", Logger::Severity::WARNING);
		XPLMDebugString(("Operation '" + operation + "' was not found\n").c_str());
		return 0x02;
	}

	XPLMDebugString(("Operation '" + operation + "' founded in callback\n").c_str());
	m_logger.Log("Operation '" + operation + "' founded in callback");

	int res = m_callbacks->at(operation)(jsonData, this);
	
	XPLMDebugString(("Operation '" + operation + "' executed and returned code : '" + std::to_string(res) + "'\n").c_str());
	m_logger.Log("Operation '" + operation + "' executed and returned code : '" + std::to_string(res) + "'");
	
	return res;
}
