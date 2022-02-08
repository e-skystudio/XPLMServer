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

#include "UDPServer.h"

using json = nlohmann::json;

static json PluginConfiguration;
static CallbackManager* callbackManager;
static int counter = 0;
float InitializerCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float NetworkCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float ExportSubscribedDataref(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
static UDPServer* server;
static Logger logger;
static std::vector<Client> clients;

void BroadCastData(std::string data)
{
	for (const Client c : clients)
	{
		server->SendData(data, c);
	}

}

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
	logger = Logger("XPLMServer.log", "[XPLMServer]", true);
	#ifdef WIN
	auto data = loadFile(".\\Resources\\plugins\\XPLMServer\\pluginConfig.json");
	#else
	auto data = loadFile("./Resources/plugins/XPLMServer/pluginConfig.json");
	#endif
	if (data.str().length() < 1)
	{
		XPLMDebugString("[XPLMServer]Unable to load configuration file\n");
		return 0;
	}
	XPLMDebugString("[XPLMServer]Configuration file loaded sucessfully\n");
	PluginConfiguration = json::parse(data.str());
	XPLMDebugString("[XPLMServer] 1\n");
	std::string sig = PluginConfiguration["Plugin"]["Name"].get<std::string>();
	std::string description = PluginConfiguration["Plugin"]["Description"].get<std::string>() ;
	XPLMDebugString("[XPLMServer] 2\n");


	XPLMDebugString("[XPLMServer] 3\n");
	strcpy(outName, sig.c_str());
	strcpy(outSig, "eskystudio.tools.XPLMServer");
	strcpy(outDesc, description.c_str());
	XPLMDebugString("[XPLMServer] 4\n");
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
	logger.Log("[XPLMServer]Enabled");
	std::string configuration;
	#ifndef _DEBUG
		configuration = "Release";
	#else
		configuration = "Debug";
	#endif
	std::string platform;
	#ifdef WIN
		#ifndef WIN64
			platform = "Win32";
		#else
			platform = "Win64";
		#endif
	#elif LIN
		platform = "Linux64";
	#elif APL
		platform = "Mac64";
	#endif
	logger.Log("Loading configuration :'" + configuration + "' & platform : '" + platform + "'");
	std::string dllPath = PluginConfiguration["DLLFiles"][platform][configuration].get<std::string>();
	logger.Log("Trying to load dll from path : '" + dllPath + "'");
	logger.Log("Creating a callback manager...");
	callbackManager = new CallbackManager();
	logger.Log("Creating a callback manager...[DONE]\n");
	logger.Log("Loading the dlls");
	int res = callbackManager->LoadCallbackDLL(dllPath);
	std::stringstream debug;
	debug << "[XPLMServer]Loading callback from DLL returned " << res << "\n Dll Path was: " << dllPath << "\n";
	logger.Log(debug.str());
	logger.Log("---Server Init----");
	if (PluginConfiguration.contains("Server") &&
		PluginConfiguration["Server"].contains("InIp") &&
		PluginConfiguration["Server"].contains("InPort"))
	{
		logger.Log("Creating server");
		server = new UDPServer();
		logger.Log("initlaizating server : " + std::to_string(PluginConfiguration["Server"]["InPort"].get<int>()));
		res = server->Bind(PluginConfiguration["Server"]["InPort"].get<unsigned short>());
		if (res == EXIT_SUCCESS)
		{
			logger.Log("Initalization sucess");
			XPLMRegisterFlightLoopCallback(InitializerCallback, -1.0f, nullptr);
		}
		else {
			logger.Log("Initalization failed, Res was " + std::to_string(res));
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
		BroadCastData(ops.dump());
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
	XPLMRegisterFlightLoopCallback(ExportSubscribedDataref, -1.0f, nullptr);
 	return 0.0f;
}

float NetworkCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	callbackManager->ExecuteConstantDataref();
	Client cli;
	std::string data = server->ReceiveData(4096, &cli);
	if (data.length() < 1)
		return -1.0f;
	logger.Log("Data Received : '" + data + "' started!");
	json operation = json::parse(data);
	callbackManager->ExecuteCallback(&operation);
	logger.Log("Data Received : '" + data + "' done!");
	server->SendData(operation.dump(), cli);
	
	bool foundClient = false;
	for (const Client c : clients)
	{
		if (c.ip == cli.ip && c.port == cli.port)
		{
			foundClient = true;
			break;
		}
	}

	if (!foundClient)
	{
		clients.push_back(cli);
	}
	return -1.0f;
}

float ExportSubscribedDataref(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	if (callbackManager->GetSubscribedDatarefCount() < 1)
	{
		return 1.0f;
	}
	auto* p_subscribedDatarefMap = callbackManager->GetSubscribedDataref();
	json jdataOut = {
		{"Operation", "ExportDataref"},
		{"Datarefs", json::array()}
	};
	std::map<std::string, Dataref*> subDatarefMap = *(callbackManager->GetSubscribedDataref());
	logger.Log("Exporting Datarefs : \n\n");
	for (auto &kv : *p_subscribedDatarefMap)
	{
		json jdataref = {
			{"Name", kv.first},
			{"Value", kv.second->GetValue()}
		};
		logger.Log(jdataref["Name"].get<std::string>() + " = " + jdataref["Value"].get<std::string>());
		jdataOut["Datarefs"].push_back(jdataref);
	}
	BroadCastData(jdataOut.dump());
	return 0.25f;
}