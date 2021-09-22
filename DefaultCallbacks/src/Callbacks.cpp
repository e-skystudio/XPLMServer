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

int RegisterDataref(json* jdata, CallbackManager* callbackManager)
{
	return 0;
}

int UnregisterDataref(json* jdata, CallbackManager* callbackManager)
{
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

int GetRegisterDatarefValue(json* jdata, CallbackManager* callbackManager)
{
	return 0;
}

int SetRegisterDatarefValue(json* jdata, CallbackManager* callbackManager)
{
	return 0;
}

int GetDatarefValue(json* jdata, CallbackManager* callbackManager)
{
	return 0;
}

int SetDatarefValue(json* jdata, CallbackManager* callbackManager)
{
	return 0;
}
