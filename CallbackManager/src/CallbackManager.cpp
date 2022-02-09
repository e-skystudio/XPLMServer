#include "../include/CallbackManager.h"

CallbackManager::CallbackManager() :
	m_logger(Logger("XPLMServer.log", "CallbackManager", false)),
	m_subscirbeDatarefCount(0)
{
	m_callbacks = new std::map<std::string, Callback>();
	m_namedDatarefs = new std::map<std::string, Dataref*>();
	m_subscribedDatarefs = new std::map<std::string, Dataref*>();
	m_constDataref = new std::vector<ConstantDataref>();
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
	#ifdef IBM
		FreeLibrary(m_hDLL);
	#else
		dlclose(m_hDLL);
	#endif
}

int CallbackManager::AppendCallback(std::string name, Callback newCallback)
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
	#ifdef IBM
		m_hDLL = LoadLibrary(s2ws(inDllPath).c_str());
	#else
		m_hDLL = dlopen(inDllPath.c_str(), RTLD_LAZY);
	#endif
	if (m_hDLL == nullptr)
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
	CallbackLoader loader;
	#ifdef IBM
		loader = reinterpret_cast<CallbackLoader>(GetProcAddress(m_hDLL, "GetCallbacks"));
	#else
		loader = reinterpret_cast<CallbackLoader>(dlsym(m_hDLL, "GetCallbacks"));
	#endif
	if(loader == nullptr)
	{
		m_logger.Log("Unable to load GetCallbacks!", Logger::Severity::CRITICAL);
		#ifndef IBM
			XPLMDebugString("[XPLM]There was an error loading the callback, error provided in XPLM logFile\n");
			char* error;
			error = dlerror();
			m_logger.Log(std::string(error), Logger::Severity::CRITICAL);
			XPLMDebugString(error);
			XPLMDebugString("\n\n");
		#endif
		return 0x02;
	}
	int size = 0;
	loader(nullptr, &size);

	ss = std::stringstream();
	ss << "There is/are " << size << " callback(s) loadable";
	m_logger.Log(ss.str().c_str());

	m_logger.Log("Creating an array of CallbackFunction...");
	std::vector<CallbackFunctionStruct*> vec_callbacks;
	m_logger.Log("Creating an array of CallbackFunction...[DONE]\n");
	m_logger.Log("Loading the callbacks...");
	loader(&vec_callbacks, &size);
	m_logger.Log("Loading the callbacks...[DONE]\n");
	
	for (std::size_t i(0); i < vec_callbacks.size(); i++)
	{
		m_logger.Log(("\nLoading callback " + std::to_string(i) + " / " + std::to_string(size - 1)).c_str());
		CallbackFunctionStruct* callback1 = vec_callbacks[i];
		m_logger.Log(("Trying to load '" + callback1->function + "' as '" + callback1->operation + "'...").c_str());
		Callback p_callback;
		#ifdef IBM
		 	p_callback = reinterpret_cast<Callback>(GetProcAddress(m_hDLL, callback1->function.c_str()));
		#else
			p_callback = reinterpret_cast<Callback>(dlsym(m_hDLL, callback1->function.c_str()));
		#endif
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

int CallbackManager::GetSubscribedDatarefCount()
{
	return m_subscirbeDatarefCount;
}

void CallbackManager::AddSubscribedDataref(std::string name)
{
	if (m_namedDatarefs == nullptr || !m_namedDatarefs->contains(name))
	{
		m_logger.Log("namedDataref is unitialised or don't contains : '" + name + "'", Logger::Severity::WARNING);
		return;
	}
	m_subscribedDatarefs->emplace(name, m_namedDatarefs->at(name));
	m_logger.Log("namedDataref : '" + name + "' founded an added to the map!");
	m_subscirbeDatarefCount++;
	m_logger.Log("There is/are " + std::to_string(m_subscirbeDatarefCount) +  " dataref subscribed");
}

void CallbackManager::RemoveSubscribedDataref(std::string name)
{
	if (m_namedDatarefs == nullptr || !m_subscribedDatarefs->contains(name))
	{
		m_logger.Log("m_subscribedDatarefs is unitialised or don't contains : '" + name + "'", Logger::Severity::WARNING);
		return;
	}
	m_subscribedDatarefs->erase(name);
	m_logger.Log("m_subscribedDatarefs : '" + name + "' founded an removed from the map!");
	m_subscirbeDatarefCount--;
	m_logger.Log("There is/are " + std::to_string(m_subscirbeDatarefCount) + " dataref subscribed");
}

int CallbackManager::GetConstantDatarefCount()
{
	return (int)m_constDataref->size();
}

void CallbackManager::AddConstantDataref(std::string name, std::string value)
{
	if (m_namedDatarefs == nullptr || !m_namedDatarefs->contains(name))
	{
		m_logger.Log("namedDataref is unitialised or don't contains : '" + name + "'", Logger::Severity::WARNING);
		return;
	}
	ConstantDataref dr;
	dr.name = name;
	dr.value = value;
	dr.dataref = m_namedDatarefs->at(name);

	m_constDataref->push_back(dr);
	m_logger.Log("namedDataref : '" + name + "' founded an added to the map!");
	m_subscirbeDatarefCount++;
	m_logger.Log("There is/are " + std::to_string(m_subscirbeDatarefCount) + " dataref(s) set as contant");
}

void CallbackManager::RemoveConstantDataref(std::string name)
{
	for (auto it = m_constDataref->begin(); it != m_constDataref->end(); ) {
		if ((*it).name == name) {
			it = m_constDataref->erase(it);
		}
		else {
			++it;
		}
	}
}

void CallbackManager::ExecuteConstantDataref()
{
	for (auto it = m_constDataref->begin(); it != m_constDataref->end(); it++)
	{
		it->dataref->SetValue(it->value);
	}
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
