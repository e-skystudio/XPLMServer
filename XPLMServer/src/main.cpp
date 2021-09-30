#include "TCPServer.h"
#include <sstream>
#include <string>

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>
#include <XPLMMenus.h>

#include <Dataref.h>
#include <CallbackManager.h>
#include <utils.h>


#include <nlohmann/json.hpp>
//#include <CallbackManager/src/utils.cpp>

using json = nlohmann::json;

static json PluginConfiguration;
static CallbackManager* callbackManager;
static Dataref visibility;
static int counter = 0;
float InitializerCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float NetworkCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
static int g_menu_container_idx; // The index of our menu item in the Plugins menu
static XPLMMenuID g_menu_id; // The menu container we'll append all our menu items to
void menu_handler_callback(void*, void*);
static TCPServer* server;

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

	g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "XPLM Server Debug", 0, 0);
	g_menu_id = XPLMCreateMenu("XPLM Server Debug", XPLMFindPluginsMenu(), 
		g_menu_container_idx, menu_handler_callback, NULL);
	XPLMAppendMenuItem(g_menu_id, "Set CAVOK", (void*)"1", 1);
	XPLMAppendMenuItem(g_menu_id, "Set LVO", (void*)"2", 1);
	XPLMAppendMenuItem(g_menu_id, "Set 0% Rain", (void*)"3", 1);
	XPLMAppendMenuItem(g_menu_id, "Set 100% Rain", (void*)"4", 1);
	XPLMAppendMenuItem(g_menu_id, "Get Visibility", (void*)"5", 1);
	XPLMAppendMenuItem(g_menu_id, "Get Rain", (void*)"6", 1);
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
	std::string configuration;
	#ifndef _DEBUG
		configuration = "Release";
	#else
		configuration = "Debug";
	#endif
	std::string platform;
	#ifndef WIN64
	platform = "Win32";
	#else
	platform = "Win64";
	#endif
	std::string dllPath = PluginConfiguration["DLLFiles"][platform][configuration].get<std::string>();
	
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
	XPLMDebugString("\n---Server Init----\n");
	if (PluginConfiguration.contains("Server") &&
		PluginConfiguration["Server"].contains("InIp") &&
		PluginConfiguration["Server"].contains("InIp"))
	{
		XPLMDebugString("[XPLMDebugString] Creating server\n");
		server = new TCPServer();
		XPLMDebugString(("[XPLMDebugString] initlaizating server : " + PluginConfiguration["Server"]["InIp"].get<std::string>() + 
		" : " + std::to_string(PluginConfiguration["Server"]["InPort"].get<int>()) + "\n").c_str());
		res = server->Initialize(PluginConfiguration["Server"]["InIp"].get<std::string>(),
			PluginConfiguration["Server"]["InPort"].get<int>());
		XPLMDebugString(("Server::Initalize returned " + std::to_string(res) + "\n").c_str());
		if (res == EXIT_SUCCESS)
		{
			XPLMDebugString("[XPLMDebugString] Initalization sucess\n");
			XPLMRegisterFlightLoopCallback(InitializerCallback, -1.0f, nullptr);
		}
		else {
			XPLMDebugString("[XPLMDebugString] Initalization failed\n");
			XPLMDebugString(("[XPLMServer] res was " + std::to_string(res) + "\n").c_str());
		}
	}

	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) 
{
	XPLMSpeakString(("Message received : '" + std::to_string(inMsg) + "'").c_str());
	if (callbackManager->GetSubscribedEventMap()->contains((unsigned int)inMsg))
	{
		json ops;
		ops["Operation"] = "Event Triggered";
		ops["Value"] = callbackManager->GetSubscribedEventMap()->at((unsigned int)inMsg);
		server->BroadcastData(ops.dump());
	}
}

float InitializerCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	json data;
	data["Operation"] = "REG_DATA";
	data["Name"] = "VISIBILITY";
	data["Link"] = "sim/weather/visibility_reported_m";
	int res = callbackManager->ExecuteCallback(&data);
	XPLMDebugString(("[XPLMServer] CallbackManager::ExecuteCallback returned :'" + std::to_string(res) + "'\n").c_str());
	XPLMRegisterFlightLoopCallback(NetworkCallback, -1.0f, nullptr);
 	return 0.0f;
}

float NetworkCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	std::vector<std::string> datas = server->RunOnce();
	for (auto const& data : datas)
	{
		json operation = json::parse(data);
		callbackManager->ExecuteCallback(&operation);
		server->BroadcastData(operation.dump());
	}
	return 0.1f;
}

void menu_handler_callback(void* in_menu_ref, void* in_item_ref)
{
	json operation;
	if (!strcmp((const char*)in_item_ref, "1"))
	{
		operation["Operation"] = "SET_REG_DATA";
		operation["Name"] = "VISIBILITY";
		operation["Value"] = "10000.0";
	}
	else if (!strcmp((const char*)in_item_ref, "2"))
	{
		operation["Operation"] = "SET_REG_DATA";
		operation["Name"] = "VISIBILITY";
		operation["Value"] = "500.0";
	}
	else if (!strcmp((const char*)in_item_ref, "3"))
	{
		operation["Operation"] = "SET_DATA";
		operation["Link"] = "sim/weather/rain_percent";
		operation["Value"] = "0.0";
	}
	else if (!strcmp((const char*)in_item_ref, "4"))
	{
		operation["Operation"] = "SET_DATA";
		operation["Link"] = "sim/weather/rain_percent";
		operation["Value"] = "1.0";
	}
	else if (!strcmp((const char*)in_item_ref, "5"))
	{
		operation["Operation"] = "GET_REG_DATA";
		operation["Name"] = "VISIBILITY";
		int res = callbackManager->ExecuteCallback(&operation);
		callbackManager->Log("Execution returned " + std::to_string(res));
		XPLMSpeakString(("Visibility : " + operation["Value"].get<std::string>() + " meters").c_str());
		return;
	}
	else if (!strcmp((const char*)in_item_ref, "6"))
	{
		operation["Operation"] = "GET_DATA";
		operation["Link"] = "sim/weather/rain_percent";
		int res = callbackManager->ExecuteCallback(&operation);
		callbackManager->Log("Execution returned " + std::to_string(res));
		XPLMSpeakString(("Rain : " + operation["Value"].get<std::string>() + " percent").c_str());
		return;
	}
	int res = callbackManager->ExecuteCallback(&operation);
	callbackManager->Log("Execution returned " + std::to_string(res));
}