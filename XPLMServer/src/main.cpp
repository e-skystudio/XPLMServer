#include <sstream>

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#include "Dataref.h"

static Dataref visibility;
float InitlaisedCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
	strcpy(outName, "XPLMServer 1.0.0.0");
	strcpy(outSig, "eskystudio.tools.XPLMServer");
	strcpy(outDesc, "A Server for XPlane");
	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int  XPluginEnable(void)
{ 
	visibility = Dataref();
	visibility.Load("sim/weather/visibility_reported_m");
	XPLMRegisterFlightLoopCallback(InitlaisedCallback, -1.0f, NULL);
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) 
{
}

float InitlaisedCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	std::stringstream ss("Visibility is ");
	ss << visibility.GetValue() << " meters";
	XPLMSpeakString(ss.str().c_str());
	visibility.SetValue("2500.0");
	return 5.0f;
}
