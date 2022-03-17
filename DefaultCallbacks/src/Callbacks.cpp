#include "Callbacks.h"

std::string ExtractJsonValue(json* jdata, std::string fieldname, CallbackManager* callback)
{
	callback->Log("ExtractJsonValue Started !");
	std::string value;
	switch (jdata->at("Value").type())
	{
	case json::value_t::string:
		value = jdata->at("Value").get<std::string>();
		callback->Log("Type is string");
		break;
	case json::value_t::number_float:
		value = std::to_string(jdata->at("Value").get<float>());
		callback->Log("Type is float");
		break;
	case json::value_t::number_integer:
		value = std::to_string(jdata->at("Value").get<int>());
		callback->Log("Type is integer");
		break;
	case json::value_t::number_unsigned:
		value = std::to_string(jdata->at("Value").get<unsigned int>());
		callback->Log("Type is unsigned integer");
		break;
	case json::value_t::object:
		value = jdata->at("Value").dump();
		callback->Log("Type is json object");
		break;
	case json::value_t::array:
		value = jdata->at("Value").dump();
		callback->Log("Type is array");
		break;
	default:
		callback->Log("Type is NOT supported ('" + std::to_string((int)jdata->at("Value").type()) + "')", Logger::Severity::CRITICAL);
		break;
	}
	if (value.length() > 0)
	{
		callback->Log("Value : '" + value + "'");
	}
	callback->Log("ExtractJsonValue END !");
	return value;
}

void GetCallbacks(std::vector<CallbackFunctionStruct*>* callbacks, int* size)
{
	*size = 0;
	if (callbacks != nullptr)
	{
		callbacks->push_back(new CallbackFunctionStruct("LOAD_DLL", "LoadDll"));
		callbacks->push_back(new CallbackFunctionStruct("REG_DATA", "RegisterDataref"));
		callbacks->push_back(new CallbackFunctionStruct("UNREG_DATA", "UnregisterDataref"));
		callbacks->push_back(new CallbackFunctionStruct("SUB_DATA", "SubscribeDataref"));
		callbacks->push_back(new CallbackFunctionStruct("UNSUB_DATA", "UnsubscribeDataref"));
		callbacks->push_back(new CallbackFunctionStruct("GET_REG_DATA", "GetRegisterDatarefValue"));
		callbacks->push_back(new CallbackFunctionStruct("SET_REG_DATA", "SetRegisterDatarefValue"));
		callbacks->push_back(new CallbackFunctionStruct("GET_DATA", "GetDatarefValue"));
		callbacks->push_back(new CallbackFunctionStruct("SET_DATA", "SetDatarefValue"));
		callbacks->push_back(new CallbackFunctionStruct("SPEAK", "Speak"));
		callbacks->push_back(new CallbackFunctionStruct("ADD_CONST", "AddConstantDataref"));
		callbacks->push_back(new CallbackFunctionStruct("LOAD_REG_DATA", "LoadRegisterDataref"));
		*size = (int)callbacks->size();
	}
	return;
}

int LoadDll(json* jdata, CallbackManager* callbackManager)
{
	callbackManager->LoadCallbackDLL(jdata->at("DLLPath").get<std::string>());
	return 0;
}

int RegisterDataref(json* jdata, CallbackManager* callback)
{
	callback->Log("RegisterDataref [START]");
	if (!jdata->contains("Name") || !jdata->contains("Link"))
	{
		callback->Log("Name and or Link missing in JSON, abording",
			Logger::Severity::CRITICAL);
		callback->Log("RegisterDataref [DONE]");
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
		callback->Log("ConversionFactor = " + conversionFactor + "\n", Logger::Severity::WARNING);
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
	callback->Log("RegisterDataref [DONE]");
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
		callback->RemoveSubscribedDataref(name);
	}
	auto beforeSize = p_datarefMap->size();
	auto afterSize = p_datarefMap->erase(name);
	callback->Log("Dataref Map sized from " + std::to_string(beforeSize) + " to " + std::to_string(afterSize));
	return 0;
}

int SubscribeDataref(json* jdata, CallbackManager* callback)
{
	callback->Log("SubscribeDataref [START]");
	if (!jdata->contains("Name"))
	{
		callback->Log("Missing mandatory JSON parameter 'Name'", Logger::Severity::CRITICAL);
		callback->Log("SubscribeDataref [DONE]");
		return 0x01;
	}
	callback->AddSubscribedDataref(jdata->at("Name").get<std::string>());
	callback->Log("SubscribeDataref [DONE]");
	return 0;
}

int UnsubscribeDataref(json* jdata, CallbackManager* callback)
{
	callback->Log("UnsubscribeDataref [START]");
	if (!jdata->contains("Name"))
	{
		callback->Log("Missing mandatory JSON parameter 'Name'", Logger::Severity::CRITICAL);
		callback->Log("UnsubscribeDataref [DONE]");
		return 0x01;
	}
	callback->RemoveSubscribedDataref(jdata->at("Name").get<std::string>());
	callback->Log("UnsubscribeDataref [DONE]");
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
	jdata->operator[]("Value") = val;
	callback->Log("Value added to json");
	return 0;
}

int SetRegisterDatarefValue(json* jdata, CallbackManager* callback)
{
	callback->Log("Executing SetRegisterDataref", Logger::Severity::TRACE);
	if (!jdata->contains("Name") || !jdata->contains("Value"))
	{
		callback->Log("Name and/or Value properties missing from JSON", Logger::Severity::CRITICAL);
		return 0x01;
	}
	std::string name = jdata->at("Name").get<std::string>();
	std::string value = ExtractJsonValue(jdata, "Value", callback);
	if (value.length() <= 0)
	{
		callback->Log("ExtractJsonValue, returned an error !");
		return 0x03;
	}

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
	Dataref* p_dataref = p_datarefMap->at(name);
	if(p_dataref == nullptr)
	{
		callback->Log("Dataref pointer is null");
		return 0x03;
	}
#ifdef _DEBUG
	callback->Log("Obtaining Dataref '" + name + "'...[DONE]");
#endif
	p_dataref->SetValue(value);
	return 0;
}

int GetDatarefValue(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Link") || jdata->at("Link").type() != json::value_t::string)
	{
		callback->Log("Link propertie missing from JSON or the type is not string", Logger::Severity::CRITICAL);
		return 0x01;
	}
	std::string link = jdata->at("Link").get<std::string>();
	callback->Log("Will be reading dataref at location :'" + link + "'");
	auto p_dataref = new Dataref();
	p_dataref->Load(link);
	if (!jdata->contains("Type") || jdata->at("Type").type() != json::value_t::string)
	{
		Dataref::Type type = p_dataref->LoadType();
		callback->Log("Dataref is of type '" + std::to_string((int)type) + "'");
	}
	else {
		p_dataref->SetType(jdata->at("Type").get<std::string>());
	}
	std::string val = p_dataref->GetValue();
	callback->Log("Value is '" + val + "'");
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
	std::string value = ExtractJsonValue(jdata, "Value", callback);
	if (value.length() <= 0)
	{
		callback->Log("ExtractJsonValue, returned an error !");
		return 0x03;
	}
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

int Speak(json* jdata, CallbackManager* callback)
{
	if(!jdata->contains("Text"))
		return 0x01;
	XPLMSpeakString(jdata->at("Text").get<std::string>().c_str());
	return 0;
}

int AddConstantDataref(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name") || !jdata->contains("Value"))
	{
		return 0x01;
	}
	callback->AddConstantDataref(jdata->at("Name").get<std::string>(), jdata->at("Value").get<std::string>());
	return 0;
}

int LoadRegisterDataref(json* jdata, CallbackManager* callback)
{
	std::ifstream csv_in;
	csv_in.open(jdata->at("FileIn").get<std::string>());
	if (!csv_in.is_open())
	{
		;
		callback->Log("Error while opening file!\n", Logger::Severity::WARNING);
		return 0x01;
	}
	std::string line;
	std::vector<std::vector<std::string>> tokens;
	while (std::getline(csv_in, line))
	{
		if (line[0] == '#')
		{
			continue;
		}
		std::vector<std::string> vecOut;
		std::size_t pos;
		while ((pos = line.find(';')) != std::string::npos)
		{
			std::string sub = line.substr(0, pos);
			vecOut.push_back(sub);
			line = line.substr(pos + 1);
		}
		Dataref* dataref = new Dataref();
		dataref->Load(vecOut[1]);
		dataref->SetType(vecOut[2]);
		dataref->SetConversionFactor(vecOut[3]);
		auto p_datarefMap = callback->GetNamedDataref();
		p_datarefMap->emplace(vecOut[0], dataref);
		callback->AddSubscribedDataref(vecOut[0]);
	}
	return 0x00;
}