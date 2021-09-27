#include "Callbacks.h"

void GetCallbacks(std::vector<CallbackFunction*>* callbacks, int* size)
{
	*size = CallbackNumber;
	if (callbacks != nullptr)
	{
		callbacks->push_back(new CallbackFunction("VISIBILITY", "SetVisibility"));
		callbacks->push_back(new CallbackFunction("LOAD_DLL", "LoadDll"));
		callbacks->push_back(new CallbackFunction("REG_DATA", "RegisterDataref"));
		callbacks->push_back(new CallbackFunction("UNREG_DATA", "UnregisterDataref"));
		callbacks->push_back(new CallbackFunction("SUB_DATA", "SubscribeDataref"));
		callbacks->push_back(new CallbackFunction("UNSUB_DATA", "UnsubscribeDataref"));
		callbacks->push_back(new CallbackFunction("GET_REG_DATA", "GetRegisterDatarefValue"));
		callbacks->push_back(new CallbackFunction("SET_REG_DATA", "SetRegisterDatarefValue"));
		callbacks->push_back(new CallbackFunction("GET_DATA", "GetDatarefValue"));
		callbacks->push_back(new CallbackFunction("SET_DATA", "SetDatarefValue"));
	}
	return;
}

int SetVisibility(json* jdata, CallbackManager* callbackManager)
{
	Dataref vis;
	vis.Load("sim/weather/visibility_reported_m");
	vis.SetValue(jdata->at("Value").get<std::string>());
	return 0;
}

int LoadDll(json* jdata, CallbackManager* callbackManager)
{
	callbackManager->LoadCallbackDLL(jdata->at("DLLPath").get<std::string>());
	return 0;
}

int RegisterDataref(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name") || !jdata->contains("Link"))
	{
		callback->Log("Name and or Link missing in JSON, abording",
			Logger::Severity::CRITICAL);
		return 0x01;
	}
	std::string link = jdata->at("Link").get<std::string>();
	std::string name = jdata->at("Name").get<std::string>();
	callback->Log("Loading dataref '" + link + "' as '" + name + "'");

	Dataref* dataref = new Dataref();
	dataref->Load(link);

	if (!jdata->contains("Type"))
	{
		callback->Log("Type was not present in JSON data, using XPlane SDK to determine it");
		Dataref::Type dType = dataref->LoadType();
		callback->Log("Dataref Type is " + std::to_string((int)dType));
	}
	else
	{
		std::string type = jdata->at("Type").get<std::string>();
		callback->Log("Dataref type is '" + type + "'");
		dataref->SetType(type);
	}
	if (jdata->contains("ConversionFactor"))
	{
		std::string conversionFactor = jdata->at("ConversionFactor").get<std::string>();
		callback->Log("ConversionFactor not yet implemented in Dataref, assuming 1.0f", Logger::Severity::WARNING);
	}
	else {
		callback->Log("ConversionFactor was not provided assuming 1.0f", Logger::Severity::WARNING);
	}
	auto p_datarefMap = callback->GetNamedDataref();
	callback->Log("Adding dataref to map", Logger::Severity::DEBUG);
	auto sizeBefore = p_datarefMap->size();
	callback->Log("Size before operation : " + std::to_string(sizeBefore), Logger::Severity::DEBUG);
	p_datarefMap->emplace(name, dataref);
	auto sizeAfter = p_datarefMap->size();
	callback->Log("Size after operation : " + std::to_string(sizeAfter), Logger::Severity::DEBUG);
	return 0;
}

int UnregisterDataref(json* jdata, CallbackManager* callback)
{
	/*TODO : 
	*	- Test implementation, this will required to implement a menu.
	* allowing to register or unregister specific dataref.
	*/
	std::string name;
	if (!jdata->contains("Name"))
	{
		callback->Log("Missing mandatory JSON parameter 'Name'", Logger::Severity::CRITICAL);
		return 0x01;
	}
	name = jdata->at("Name").get<std::string>();
	callback->Log("Request deletion of dataref '" + name + "'");
	auto p_datarefMap = callback->GetNamedDataref();
	if (!p_datarefMap->contains(name))
	{
		callback->Log("'" + name + "' no found in registered dataref");
		return 0x02;
	}
	auto p_subscribeDatarefMap = callback->GetSubscribedDataref();
	if (p_subscribeDatarefMap->contains(name))
	{
		callback->Log("'" + name + "' was also found in subscribe dataref : DELETING");
	}
	auto beforeSize = p_datarefMap->size();
	auto afterSize = p_datarefMap->erase(name);
	callback->Log("Dataref Map sized from " + std::to_string(beforeSize) + " to " + std::to_string(afterSize));
	return 0;
}

int SubscribeDataref(json* jdata, CallbackManager* callbackManager)
{
	return 0;
}

int UnsubscribeDataref(json* jdata, CallbackManager* callbackManager)
{
	return 0;
}

int GetRegisterDatarefValue(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name"))
	{
		callback->Log("Name properties missing from JSON", Logger::Severity::CRITICAL);
		return 0x01;
	}
	std::string name = jdata->at("Name").get<std::string>();
	callback->Log("Looking for dataref '" + name + "' to get value");
	auto p_datarefMap = callback->GetNamedDataref();
	callback->Log("Obtaining the registered datarefs...[DONE]");
	if (!p_datarefMap->contains(name))
	{
		callback->Log("Registered Dataref don't contain '" + name + "'", Logger::Severity::DEBUG);
		return 0x02;
	}
#ifdef _DEBUG
	callback->Log("Dataref '" + name + "' found!");
#endif
	auto p_dataref = p_datarefMap->at(name);
#ifdef _DEBUG
	callback->Log("Obtaining Dataref '" + name + "'...[DONE]");
#endif
	std::string val = p_dataref->GetValue();
	callback->Log("Value is '" + val + "'");
	//BUG
	jdata->operator[]("Value") = val;
	//jdata->push_back(std::pair<std::string, std::string>("Value", val));
	callback->Log("Value added to json");
	//!BUG
	return 0;
}

int SetRegisterDatarefValue(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name") || !jdata->contains("Value"))
	{
		callback->Log("Name and/or Value properties missing from JSON", Logger::Severity::CRITICAL);
		return 0x01;
	}
	std::string name = jdata->at("Name").get<std::string>();
	std::string value = jdata->at("Value").get<std::string>();
	callback->Log("Looking for dataref '" + name + "' to set value to : '" + value + "'");

	auto p_datarefMap = callback->GetNamedDataref();
	callback->Log("Obtaining the registered datarefs...[DONE]");
	if (!p_datarefMap->contains(name))
	{
		callback->Log("Registered Dataref don't contain '" + name +"'", Logger::Severity::DEBUG);
		return 0x02;
	}
#ifdef _DEBUG
	callback->Log("Dataref '"+ name + "' found!");
#endif
	auto p_dataref = p_datarefMap->at(name);
#ifdef _DEBUG
	callback->Log("Obtaining Dataref '" + name + "'...[DONE]");
#endif
	p_dataref->SetValue(value);
	return 0;
}

int GetDatarefValue(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Link"))
	{
		callback->Log("Link propertie missing from JSON", Logger::Severity::CRITICAL);
		return 0x01;
	}
	std::string link = jdata->at("Link").get<std::string>();
	callback->Log("Will be reading dataref at location :'" + link + "'");
	auto p_dataref = new Dataref();
	p_dataref->Load(link);
	if (!jdata->contains("Type"))
	{
		Dataref::Type type = p_dataref->LoadType();
		callback->Log("Dataref is of type '" + std::to_string((int)type) + "'");
	}
	else {
		p_dataref->SetType(jdata->at("Type").get<std::string>());
	}
	std::string val = p_dataref->GetValue();
	callback->Log("Value is '" + val + "'");
	//jdata->push_back(std::pair<std::string, std::string>("Value", val));
	jdata->operator[]("Value") = val;
	callback->Log("Value added to json");
	return 0;
}

int SetDatarefValue(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Link") || !jdata->contains("Value"))
	{
		callback->Log("Value and/or Link propertie(s) missing from JSON", Logger::Severity::CRITICAL);
		return 0x01;
	}
	std::string value = jdata->at("Value").get<std::string>();
	std::string link = jdata->at("Link").get<std::string>();
	callback->Log("Will be setting dataref at location :'" + link + "' to value + :'" + value + "'");
	auto p_dataref = new Dataref();
	p_dataref->Load(link);
	if (!jdata->contains("Type"))
	{
		Dataref::Type type = p_dataref->LoadType();
		callback->Log("Dataref is of type '" + std::to_string((int)type) + "'");
	}
	else {
		std::string type = jdata->at("Type").get<std::string>();
		p_dataref->SetType(type);
	}
	p_dataref->SetValue(value);
	return 0;
}
