#include <sstream>
#include <string>

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#include <Dataref.h>
#include <CallbackManager.h>

static CallbackManager* callbackManager;
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
	std::string dllPath;
	#ifdef WIN64
		#ifdef _DEBUG
			dllPath = ".\\Resources\\plugins\\XPLMServer\\DefaultCallbacks_64-d.dll";
		#else
			dllPath = ".\\Resources\\plugins\\XPLMServer\\DefaultCallbacks_64.dll";
		#endif
	#else
		#ifdef _DEBUG
			dllPath = ".\\Resources\\plugins\\XPLMServer\\DefaultCallbacks-d.dll";
		#else
			dllPath = ".\\Resources\\plugins\\XPLMServer\\DefaultCallbacks.dll";
		#endif
	#endif
	callbackManager = new CallbackManager();
	int res = callbackManager->LoadCallbackDLL(dllPath);
	std::stringstream debug;
	debug << "Loading callback from DLL returned " << res << "\n Dll Path was: " << dllPath << "\n";
	XPLMDebugString(debug.str().c_str());
	XPLMSpeakString(debug.str().c_str());
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
