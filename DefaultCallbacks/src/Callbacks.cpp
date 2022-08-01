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
		callback->Log("Type is NOT supported ('" + std::to_string(static_cast<int>(jdata->at("Value").type())) + "')", Logger::Severity::CRITICAL);
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
		callbacks->push_back(new CallbackFunctionStruct("FFDATA_INIT", "InitFlightFactorA320"));
		callbacks->push_back(new CallbackFunctionStruct("REG_FFDATA", "RegisterFFDataref"));
		*size = static_cast<int>(callbacks->size());
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
	if (!jdata->contains("Name") || jdata->at("Name").is_null() || !jdata->contains("Link") || jdata->at("Link").is_null())
	{
		callback->Log("Name and or Link are missings or null in JSON, abording",
			Logger::Severity::CRITICAL);
		callback->Log("RegisterDataref [DONE]");
		return 0x01;
	}
	const std::string link = jdata->at("Link").get<std::string>();
	auto name = jdata->at("Name").get<std::string>();

	auto* dataref = new Dataref();
	dataref->Load(link);

	if (!jdata->contains("Type") || jdata->at("Name").is_null() ||
		jdata->at("Type").type() != json::value_t::string || 
		jdata->at("Type").get<std::string>() == "UNKNWON")
	{
		callback->Log("Type was not present in JSON data, using XPlane SDK to determine it");
		Dataref::Type dType = dataref->LoadType();
		callback->Log("Dataref Type is " + std::to_string(static_cast<int>(dType)));
	}
	else
	{
		const std::string type = jdata->at("Type").get<std::string>();
		dataref->SetType(type);
	}
	std::string conversionFactor;
	if (jdata->contains("ConversionFactor") && !jdata->at("ConversionFactor").is_null())
	{
		conversionFactor = jdata->at("ConversionFactor").get<std::string>();
	}
	else {
		conversionFactor = "1.0f";
		callback->Log("ConversionFactor was not provided assuming 1.0f", Logger::Severity::WARNING);
	}
	dataref->SetConversionFactor(conversionFactor);
	const auto p_datarefMap = callback->GetNamedDataref();
	p_datarefMap->emplace(name, dataref);
	return 0;
}

int UnregisterDataref(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name") || jdata->at("Name").is_null())
	{
		callback->Log("Missing mandatory JSON parameter 'Name'", Logger::Severity::CRITICAL);
		return 0x01;
	}
	const std::string name = jdata->at("Name").get<std::string>();
	callback->Log("Request deletion of dataref '" + name + "'");
	//const auto p_datarefMap = callback->GetNamedDataref();
	if (!callback->GetNamedDataref()->contains(name))
	{
		callback->Log("'" + name + "' no found in registered dataref");
		return 0x02;
	}
	if (const auto p_subscribeDatarefMap = callback->GetSubscribedDataref(); p_subscribeDatarefMap->contains(name))
	{
		callback->Log("'" + name + "' was also found in subscribe dataref : DELETING");
		callback->RemoveSubscribedDataref(name);
	}
	const auto beforeSize = callback->GetNamedDataref()->size();
	const auto afterSize = callback->GetNamedDataref()->erase(name);
	callback->Log("Dataref Map sized from " + std::to_string(beforeSize) + " to " + std::to_string(afterSize));
	return 0;
}

int SubscribeDataref(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name") || jdata->at("Name").is_null())
	{
		callback->Log("SubscibeDataref : Missing mandatory JSON parameter 'Name'", Logger::Severity::CRITICAL);
		return 0x01;
	}
	callback->AddSubscribedDataref(jdata->at("Name").get<std::string>());
	return 0;
}

int UnsubscribeDataref(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name") || jdata->at("Name").is_null())
	{
		callback->Log("Missing mandatory JSON parameter 'Name'", Logger::Severity::CRITICAL);
		return 0x01;
	}
	callback->RemoveSubscribedDataref(jdata->at("Name").get<std::string>());
	return 0;
}

int GetRegisterDatarefValue(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name") || jdata->at("Name").is_null())
	{
		callback->Log("Name properties missing from JSON", Logger::Severity::CRITICAL);
		return 0x01;
	}
	std::string name = jdata->at("Name").get<std::string>();
	auto p_datarefMap = callback->GetNamedDataref();
	if (!p_datarefMap->contains(name))
	{
		callback->Log("Registered Dataref don't contain '" + name + "'", Logger::Severity::DEBUG);
		return 0x02;
	}
	auto p_dataref = p_datarefMap->at(name);
	std::string val = p_dataref->GetValue();
	callback->Log("Value is '" + val + "'");
	jdata->operator[]("Value") = val;
	callback->Log("Value added to json");
	return 0;
}

int SetRegisterDatarefValue(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name") || jdata->at("Name").is_null() || !jdata->contains("Value") || jdata->at("Value").is_null())
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
	auto p_datarefMap = callback->GetNamedDataref();
	callback->Log("Obtaining the registered datarefs...[DONE]");
	if (!p_datarefMap->contains(name))
	{
		callback->Log("Registered Dataref don't contain '" + name +"'", Logger::Severity::DEBUG);
		return 0x02;
	}
	AbstractDataref* p_dataref = p_datarefMap->at(name);
	if(p_dataref == nullptr)
	{
		callback->Log("Dataref pointer is null");
		return 0x03;
	}
	p_dataref->SetValue(value);
	return 0;
}

int GetDatarefValue(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Link") || jdata->at("Link").is_null() || jdata->at("Link").type() != json::value_t::string)
	{
		callback->Log("Link propertie missing from JSON or the type is not string", Logger::Severity::CRITICAL);
		return 0x01;
	}
	std::string link = jdata->at("Link").get<std::string>();
	callback->Log("Will be reading dataref at location :'" + link + "'");
	auto p_dataref = new Dataref();
	p_dataref->Load(link);
	if (!jdata->contains("Type") || jdata->at("Type").is_null() ||
		jdata->at("Type").type() != json::value_t::string ||
		jdata->at("Type").get<std::string>() == "UNKNWON")
	{
		Dataref::Type type = p_dataref->LoadType();
		callback->Log("Dataref is of type '" + std::to_string(static_cast<int>(type)) + "'");
	}
	else {
		p_dataref->SetType(jdata->at("Type").get<std::string>());
	}
	std::string val = p_dataref->GetValue();
	jdata->operator[]("Value") = val;
	return 0;
}

int SetDatarefValue(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Link") || jdata->at("Link").is_null() || !jdata->contains("Value") || jdata->at("Value").is_null())
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
	if(link.find("Aircraft") != std::string::npos && link.find(".") != std::string::npos)
    {
		SharedValuesInterface* ff320 = callback->GetFF320Interface();
		const auto p_dataref = new FFDataref(ff320);
		p_dataref->Load(link);
		FFDataref::Type dType = p_dataref->LoadType();
		callback->Log("FFDataref Type is " + std::to_string(static_cast<int>(dType)));
		jdata->emplace("ID", p_dataref->GetID());
		jdata->emplace("Type", p_dataref->GetType());
		jdata->emplace("Flag", p_dataref->GetFlag());
		jdata->emplace("Unit", p_dataref->GetUnit());
		jdata->emplace("Name", p_dataref->GetName());
		callback->Log("1");
		jdata->emplace("Description", p_dataref->GetDescription());
		callback->Log("2");
		p_dataref->SetValue(value);
		callback->AddFFDatarefToUpdate(p_dataref);
		callback->Log("3");
    }
	else
	{
		callback->Log("REGULAR DATAREF FOUND !");
		const auto p_dataref = new Dataref();
		p_dataref->Load(link);
		if (!jdata->contains("Type") || jdata->at("Type").is_null() ||
			jdata->at("Type").type() != json::value_t::string ||
			jdata->at("Type").get<std::string>() == "UNKNWON")
		{
			Dataref::Type type = p_dataref->LoadType();
			callback->Log("Dataref is of type '" + std::to_string(static_cast<int>(type)) + "'");
		}
		else {
			const auto type = jdata->at("Type").get<std::string>();
			p_dataref->SetType(type);
		}
		p_dataref->SetValue(value);
	}
	return 0;
}

int Speak(json* jdata, CallbackManager* callback)
{
	if(!jdata->contains("Text") || jdata->at("Text").is_null())
		return 0x01;
	XPLMSpeakString(jdata->at("Text").get<std::string>().c_str());
	return 0;
}

int AddConstantDataref(json* jdata, CallbackManager* callback)
{
	if (!jdata->contains("Name") || jdata->at("Name").is_null() || !jdata->contains("Value") || jdata->at("Value").is_null())
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

int InitFlightFactorA320(json* jdata, CallbackManager* callback)
{
	callback->Log("FFA32F INIT START");
	bool res = callback->InitFF320Interface();
	callback->Log("FFA32F INIT STOP WITH CODE : " + std::to_string(res));
	return res ? 0 : 1;
}

int RegisterFFDataref(json* jdata, CallbackManager* callback)
{
	if (!callback->IsFF320InterfaceEnabled() && !callback->InitFF320Interface())
	{
		return 0x10; //Unable to execute interface;
	}
	SharedValuesInterface* ff320 = callback->GetFF320Interface();
	callback->Log("RegisterFFDataref [START]");
	if (!jdata->contains("Name") || jdata->at("Name").is_null() || !jdata->contains("Link") || jdata->at("Link").is_null())
	{
		callback->Log("Name and or Link missing in JSON, abording",
			Logger::Severity::CRITICAL);
		callback->Log("RegisterFFDataref [DONE]");
		return 0x01;
	}
	const auto link = jdata->at("Link").get<std::string>();
	auto name = jdata->at("Name").get<std::string>();
	callback->Log("Loading FFDataref '" + link + "' as '" + name + "'");

	auto ffdataref = new FFDataref(ff320);
	ffdataref->Load(link);
	callback->Log("Loading type");
	FFDataref::Type dType = ffdataref->LoadType();
	callback->Log("FFDataref Type is " + std::to_string(static_cast<int>(dType)));
	std::string conversionFactor;
	if (jdata->contains("ConversionFactor") && !jdata->at("ConversionFactor").is_null())
	{
		conversionFactor = jdata->at("ConversionFactor").get<std::string>();
	}
	else {
		conversionFactor = "1.0f";
		callback->Log("ConversionFactor was not provided assuming 1.0f", Logger::Severity::WARNING);
	}
	ffdataref->SetConversionFactor(conversionFactor);

	jdata->emplace("ID", ffdataref->GetID());
	jdata->emplace("Type", ffdataref->GetType());
	jdata->emplace("Flag", ffdataref->GetFlag());
	jdata->emplace("Unit", ffdataref->GetUnit());
	jdata->emplace("Name", ffdataref->GetName());
	jdata->emplace("Description", ffdataref->GetDescription());

	const auto p_datarefMap = callback->GetNamedDataref();
	callback->Log("Adding FFDataref to map", Logger::Severity::DEBUG);
	p_datarefMap->emplace(name, ffdataref);
	callback->Log("RegisterFFDataref [DONE]");
	return 0;
}