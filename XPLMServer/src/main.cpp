#include <sstream>

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#include <Dataref.h>

static Dataref visibility;
float InitlaisedCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);

PLUGIN_API int XPluginStart(char* outName, char* outSignature, char* outDescription)
{
	visibility = Dataref();
	visibility.Load("sim/weather/visibility_reported_m");
	XPLMRegisterFlightLoopCallback(InitlaisedCallback, -1.0f, NULL);
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
	return 0.0f;
}
