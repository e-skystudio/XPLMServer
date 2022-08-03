// ReSharper disable CppInconsistentNaming
#include "../include/CallbackManager.h"

/*
 *-- Note -- :
 *Maybe we can create a function that will check for exisiting element and their required type.
 *Function will take a List of Key-Value pairs ("string" - "JSON Type") and a JSON object and will return
 *true if all the required keys are correct.
 */

CallbackFunctionStruct::CallbackFunctionStruct()
= default;

CallbackFunctionStruct::CallbackFunctionStruct(std::string operation, std::string functionName):
	Operation(std::move(operation)), Function(std::move(functionName))
{
}

void FF320_Callback(double step, void* tag)
{
	const auto cm = static_cast<CallbackManager*>(tag);
	cm->ExecuteConstantDataref();
	cm->ExecuteFFDatarefsUpdate();
	// for(auto &dataref : *p_datarefs)
	// {
	// 	if(dataref.second->DatarefType == DatarefType::FFDataref)
	// 	{
	// 		FFDataref* ffdata = (FFDataref*)dataref.second;
	// 		if(ffdata->NeedUpdate())
	// 		{
	// 			cm->Log(ffdata->GetName() + " need update ? " + std::to_string(ffdata->NeedUpdate()));
	// 			ffdata->DoSetValue(ffdata->GetTargetValue());
	// 		}
	// 	}
	// }
}

CallbackManager::CallbackManager() :
	m_logger(Logger("XPLMServer.log", "CallbackManager", false)),
	m_subscirbeDatarefCount(0), 
	m_hDLL(nullptr),
	m_ff320(new SharedValuesInterface())
{
	m_callbacks = new std::map<std::string, Callback>();
	m_namedDatarefs = new std::map<std::string, AbstractDataref*>();
	m_subscribedDatarefs = new std::map<std::string, AbstractDataref*>();
	m_constDataref = new std::map<std::string, ConstantDataref>();
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
		if(kv->second->DatarefType == DatarefType::XPLMDataref) delete dynamic_cast<Dataref*>(kv->second);
		// else if(kv->second->DatarefType == "FFDataref") delete (FFDataref*)(kv->second);
		// delete kv->second;
		++kv;
	}
	#ifdef IBM
		FreeLibrary(m_hDLL);
	#else
		dlclose(m_hDLL);
	#endif
}

int CallbackManager::AppendCallback(const std::string& name, Callback newCallback) const
{
	if (m_callbacks->contains(name))
	{
		return EXIT_FAILURE;
	}
	m_callbacks->emplace(name, newCallback);
	return EXIT_SUCCESS;
}


std::map<std::string, AbstractDataref*>* CallbackManager::GetNamedDataref() const
{
	return m_namedDatarefs;
}

std::map<std::string, AbstractDataref*>* CallbackManager::GetSubscribedDataref() const
{
	return m_subscribedDatarefs;
}

std::map<unsigned int, std::string>* CallbackManager::GetSubscribedEventMap() const
{
	return m_subscribedEvent;
}

int CallbackManager::LoadCallbackDLL(const std::string& inDllPath)
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
		return -1;
	}
	std::stringstream ss;
	ss << "DLL : '" << inDllPath << "' LOADED SUCESSFULLY !";
	m_logger.Log(ss.str(), Logger::Severity::TRACE);
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
	std::vector<CallbackFunctionStruct*> vec_callbacks;
	loader(&vec_callbacks, &size);
	m_logger.Log("Loading the callbacks...[DONE]\n");
	
	for (std::size_t i(0); i < vec_callbacks.size(); i++)
	{
		CallbackFunctionStruct* callback1 = vec_callbacks[i];
		Callback p_callback;
		#ifdef IBM
		 	p_callback = reinterpret_cast<Callback>(GetProcAddress(m_hDLL, callback1->Function.c_str()));
		#else
			p_callback = reinterpret_cast<Callback>(dlsym(m_hDLL, callback1->Function.c_str()));
		#endif
		if (p_callback == nullptr)
		{
			m_logger.Log("pointer callback is pointing to nullptr", Logger::Severity::WARNING);
			continue;
		}
		int res = this->AppendCallback(std::string(callback1->Operation), p_callback);
		if (res != EXIT_SUCCESS)
		{
			m_logger.Log("Appending Callback to list : [FAILED]", Logger::Severity::WARNING);
			continue;
		}
#ifdef _DEBUG
		m_logger.Log("loading of '" + std::string(callback1->Function) + "' as '" +
		             std::string(callback1->Operation) + "' sucessfull!", Logger::Severity::DEBUG);
#endif
	}
	m_logger.Log("LoadCallbackDLL...[FINISHED]");
	return size;

}

void CallbackManager::Log(const std::string& data, const Logger::Severity severity)
{
	m_logger.Log(data, severity);
}

int CallbackManager::GetSubscribedDatarefCount() const
{
	return m_subscirbeDatarefCount;
}

bool CallbackManager::AddSubscribedDataref(const std::string& name)
{
	if (m_namedDatarefs == nullptr || !m_namedDatarefs->contains(name))
	{
		m_logger.Log("namedDataref is unitialised or don't contains : '" + name + "'", Logger::Severity::WARNING);
		return false;
	}
	m_subscribedDatarefs->emplace(name, m_namedDatarefs->at(name));
	m_subscirbeDatarefCount++;
	return true;
}

bool CallbackManager::RemoveSubscribedDataref(const std::string& name)
{
	if (m_namedDatarefs == nullptr || !m_subscribedDatarefs->contains(name))
	{
		m_logger.Log("m_subscribedDatarefs is unitialised or don't contains : '" + name + "'", Logger::Severity::WARNING);
		return false;
	}
	m_subscribedDatarefs->erase(name);
	m_subscirbeDatarefCount--;
	return true;
}

int CallbackManager::GetConstantDatarefCount() const
{
	return static_cast<int>(m_constDataref->size());
}

bool CallbackManager::AddConstantDataref(const std::string& name, const std::string& value)
{
	if (m_namedDatarefs == nullptr || !m_namedDatarefs->contains(name))
	{
		m_logger.Log("namedDataref is unitialised or don't contains : '" + name + "'", Logger::Severity::WARNING);
		return false;
	}
	//m_constDataref->
	if (m_constDataref->contains(name))
	{
		m_constDataref->at(name).Value = value;
		return false;
	}
	ConstantDataref dr;
	dr.Name = name;
	dr.Value = value;
	AbstractDataref* dataref = m_namedDatarefs->at(name);
	if(dataref == nullptr) return false;
	dr.Dataref = dataref;
	m_constDataref->emplace(name, dr);
	return true;
}

bool CallbackManager::RemoveConstantDataref(const std::string& name) const
{
	if (!m_constDataref->contains(name))
	{	
		return false;
	}
	m_constDataref->erase(name);
	return true;
}

void CallbackManager::ExecuteConstantDataref() const
{
	for (auto &kv : *m_constDataref)
	{
		kv.second.Dataref->SetValue(kv.second.Value);
	}
}

int CallbackManager::ExecuteCallback(json& jsonData)
{
	if (!jsonData.contains("Operation"))
		return 0x01;

	const std::string operation = jsonData.at("Operation").get<std::string>();
	m_logger.Log("Operation '" + operation + "' was requested");

	if (!m_callbacks->contains(operation))
	{
		m_logger.Log("Operation '" + operation + "' was not found", Logger::Severity::WARNING);
		return 0x02;
	}

	m_logger.Log("Operation '" + operation + "' founded in callback");

	int res = m_callbacks->at(operation)(jsonData, *this);
	jsonData.push_back({ "Result", res });
	m_logger.Log("Operation '" + operation + "' executed and returned code : '" + std::to_string(res) + "'");
	
	return res;
}

int CallbackManager::ExecuteFFDatarefsUpdate()
{
	int updated = 0;
	while(!m_ff320_datarefs.empty())
	{
		FFDataref* dataref = m_ff320_datarefs.front();
		dataref->DoSetValue(dataref->GetTargetValue());
		free(dataref);
		m_ff320_datarefs.pop();
		updated++;
	}
	return updated;
}

void CallbackManager::AddFFDatarefToUpdate(FFDataref* dataref)
{
	m_ff320_datarefs.push(dataref);
}

SharedValuesInterface* CallbackManager::GetFF320Interface() const
{
	return m_ff320;
}

bool CallbackManager::InitFF320Interface(){
	m_logger.Log("Initalising FF320 Data Interface...");
	const int ffPluginID = XPLMFindPluginBySignature(XPLM_FF_SIGNATURE);
	if(ffPluginID == XPLM_NO_PLUGIN_ID)
	{
		m_logger.Log("Plugin not found !", Logger::Severity::CRITICAL);
		return false;
	}
	m_logger.Log("FF320 plugin ID : " + std::to_string(ffPluginID));
	XPLMSendMessageToPlugin(ffPluginID, XPLM_FF_MSG_GET_SHARED_INTERFACE, m_ff320);
	if (m_ff320->DataVersion == nullptr) {
		m_logger.Log("[FF320API] Unable to load version!");
		return false;
	}
	const unsigned int ffAPIdataversion = m_ff320->DataVersion();
	m_logger.Log("[FF320API] Version : " + std::to_string(ffAPIdataversion));

	m_ff320->DataAddUpdate(FF320_Callback, this);
	return true;
}

bool CallbackManager::IsFF320InterfaceEnabled() const
{
	return m_ff320->DataVersion != nullptr;
}

bool CallbackManager::AddNamedDataref(const std::string name, AbstractDataref* dataref)
{
	if (m_namedDatarefs == nullptr) return false;
	m_namedDatarefs->emplace(name, dataref);
	return true;
}

bool CallbackManager::RemovedNamedDataref(const std::string& name)
{
	if (!m_namedDatarefs->contains(name))
	{
		return false;
	}
	m_namedDatarefs->erase(name);
	RemoveSubscribedDataref(name);
	RemoveConstantDataref(name);
	return true;
}

AbstractDataref* CallbackManager::GetDatarefByName(const std::string& name)
{
	if (!m_namedDatarefs->contains(name))
	{
		return nullptr;
	}
	return m_namedDatarefs->at(name);
}

std::string CallbackManager::GetDatarefValue(const std::string& name)
{
	if (!m_namedDatarefs->contains(name))
	{
		return "N/A";
	}
	return m_namedDatarefs->at(name)->GetValue();
}

bool CallbackManager::SetDatarefValue(const std::string& name, const std::string& value)
{
	if (!m_namedDatarefs->contains(name))
	{
		return false;
	}
	m_namedDatarefs->at(name)->SetValue(value);
	return true;
}

