#include "Callbacks.h"

void GetCallbacks(CallbackFunction** callbacks, int* size)
{
	*size = CallbackNumber;
	if (callbacks != nullptr)
	{
		//CallbackFunction* callbacksArray = new CallbackFunction[CallbackNumber];
		*(callbacks + 0) = new CallbackFunction;
		(*(callbacks + 0))->function = "SetVisibility";
		(*(callbacks + 0))->operation = "VISIBILITY";

		*(callbacks + 1) = new CallbackFunction;
		(*(callbacks + 1))->function = "LoadDll";
		(*(callbacks + 1))->operation = "LOAD_DLL";
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
