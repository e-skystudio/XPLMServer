#include "Callbacks.h"

void GetCallbacks(CallbackFunction** callbacks, int* size)
{
	*size = CallbackNumber;
	if (callbacks != nullptr)
	{
		CallbackFunction* callbacksArray = new CallbackFunction[CallbackNumber];
		*(callbacks + 0) = new CallbackFunction;
		(*(callbacks + 0))->function = "SetVisibility";
		(*(callbacks + 0))->operation = "VISIBILITY";
		//copy callbacks
	}
	return;
}

int SetVisibility(json jdata, CallbackManager* callbackManager)
{
	Dataref vis;
	vis.Load("sim/weather/visibility_reported_m");
	vis.SetValue(jdata["Value"].get<std::string>());
	return 0;
}
