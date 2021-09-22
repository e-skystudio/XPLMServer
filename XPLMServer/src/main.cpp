#include <sstream>
#include <string>

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#include <Dataref.h>
#include <CallbackManager.h>

static CallbackManager* callbackManager;
static Dataref visibility;
static int counter = 0;
float InitializerCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float LoopCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);

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
	XPLMDebugString("[XPLMServer]Enabled");
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
	std::string dllLog = "[XPLMServer]Trying to load dll from path : '" + dllPath + "'\n";
	XPLMDebugString(dllLog.c_str());
	XPLMDebugString("[XPLMServer]Creating a callback manager...\n");
	callbackManager = new CallbackManager();
	XPLMDebugString("[XPLMServer]Creating a callback manager...[DONE]\n");
	XPLMDebugString("[XPLMServer]Loading the dlls");
	int res = callbackManager->LoadCallbackDLL(dllPath);
	XPLMDebugString("[XPLMServer]Loading the dlls [DONE]\n");
	std::stringstream debug;
	debug << "[XPLMServer]Loading callback from DLL returned " << res << "\n Dll Path was: " << dllPath << "\n";
	XPLMDebugString(debug.str().c_str());
	XPLMSpeakString(debug.str().c_str());
	XPLMDebugString("[XPLMServer]Registering callback to next display frame[DONE]\n");
	XPLMRegisterFlightLoopCallback(InitializerCallback, -1.0f, nullptr);
	XPLMRegisterFlightLoopCallback(LoopCallback, 0.0f, nullptr);
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) 
{
}

float InitializerCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	json data;
	data["Operation"] = "REG_DATA";
	data["Name"] = "VISIBILITY";
	data["Link"] = "sim/weather/visibility_reported_m";
	int res = callbackManager->ExecuteCallback(&data);
	XPLMDebugString(("[XPLMServer] CallbackManager::ExecuteCallback returned :'" + std::to_string(res) + "'\n").c_str());
	XPLMSetFlightLoopCallbackInterval(LoopCallback, 1.0, 1, nullptr);
 	return 0.0f;
}

float LoopCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	json data;
	data["Operation"] = "SET_REG_DATA";
	data["Name"] = "VISIBILITY";
	if (counter % 2 == 0)
	{
		data["Value"] = "500.0";
	}
	else
	{
		data["Value"] = "10000.0";
	}
	int res = callbackManager->ExecuteCallback(&data);
	counter++;
	return 5.0f;
}
