#include <sstream>
#include <string>

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#include <Dataref.h>
#include <CallbackManager.h>
#include <utils.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

static json PluginConfiguration;
static CallbackManager* callbackManager;
static Dataref visibility;
static int counter = 0;
float InitializerCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float LoopCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);



PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
	auto data = loadFile(".\\Resources\\plugins\\XPLMServer\\pluginConfig.json");
	if (data.str().length() < 1)
	{
		XPLMDebugString("[XPLMServer]Unable to load configuration file");
		return 0;
	}

	PluginConfiguration = json::parse(data.str());

	std::string sig = PluginConfiguration["Plugin"]["Name"].get<std::string>();
	std::string description = PluginConfiguration["Plugin"]["Description"].get<std::string>() ;


	strcpy(outName, sig.c_str());
	strcpy(outSig, "eskystudio.tools.XPLMServer");
	strcpy(outDesc, description.c_str());
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
			dllPath = PluginConfiguration["DLLFiles"]["Win64"]["Debug"];
			//            .\Resources\ plugins\ XPLMServer\ DefaultCallbacks64-d.dll
			//dllPath = ".\\Resources\\plugins\\XPLMServer\\DefaultCallbacks_64-d.dll";
		#else
			dllPath = PluginConfiguration["DLLFiles"]["Win64"]["Release"];
			//dllPath = ".\\Resources\\plugins\\XPLMServer\\DefaultCallbacks_64.dll";
		#endif
	#else
		#ifdef _DEBUG
			dllPath = PluginConfiguration["DLLFiles"]["Win32"]["Debug"];
			//dllPath = ".\\Resources\\plugins\\XPLMServer\\DefaultCallbacks-d.dll";
		#else
			dllPath = PluginConfiguration["DLLFiles"]["Win32"]["Release"];
			//dllPath = ".\\Resources\\plugins\\XPLMServer\\DefaultCallbacks.dll";
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
/* TODO:
*	Replace both callback by a menu, allowing to more precisly test callbacks implementation
*/
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
	json setData1;
	setData1["Operation"] = "SET_REG_DATA";
	setData1["Name"] = "VISIBILITY";
	json setData2;
	setData2["Operation"] = "SET_DATA";
	setData2["Link"] = "sim/weather/rain_percent";
	if (counter % 2 == 0)
	{
		setData1["Value"] = "500.0";
		setData2["Value"] = "0.0";
	}
	else
	{
		setData1["Value"] = "1.0";
		setData2["Value"] = "10000.0";
	}
	int res = callbackManager->ExecuteCallback(&setData1);
	int res2 = callbackManager->ExecuteCallback(&setData2);
	counter++;
	json getData1;
	getData1["Operation"] = "GET_REG_DATA";
	getData1["Name"] = "VISIBILITY";
	json getData2;
	getData2["Operation"] = "GET_DATA";
	getData2["Link"] = "sim/weather/rain_percent";
	
	callbackManager->ExecuteCallback(&getData1);
	callbackManager->ExecuteCallback(&getData2);
	XPLMSpeakString(("Visibility : " + getData1["Value"].get<std::string>() +
		" meters\nRain : " + getData2["Value"].get<std::string>() + " percent").c_str());
	return 5.0f;
}
