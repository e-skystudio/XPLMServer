#include "UDPServer.h" //avoid error on windows
#include <sstream>
#include <string>

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>
#include <XPLMMenus.h>

#include <Dataref.h>
#include <CallbackManager.h>
#include <utils.h>
#include <nlohmann/json.hpp>


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
	#ifdef IBM
	auto data = loadFile(".\\Resources\\plugins\\XPLMServer\\pluginConfig.json");
	#else
	auto data = loadFile("./Resources/plugins/XPLMServer/pluginConfig.json");
	#endif
	if (data.str().length() < 1)
	{
		XPLMDebugString("[XPLMServer]Unable to load configuration file\n");
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
	logger.Log("[XPLMServer]Enabled");
	std::string configuration;
	#ifndef _DEBUG
		configuration = "Release";
	#else
		configuration = "Debug";
	#endif
	std::string platform;
	#ifdef IBM
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
	callbackManager = new CallbackManager();
	int res = callbackManager->LoadCallbackDLL(dllPath);
	std::stringstream debug;
	logger.Log(debug.str());
	logger.Log("---Server Init----");
	if (PluginConfiguration.contains("Server") &&
		PluginConfiguration["Server"].contains("InIp") &&
		PluginConfiguration["Server"].contains("InPort"))
	{
		server = new UDPServer();
		res = server->Bind(PluginConfiguration["Server"]["InPort"].get<unsigned short>());
		if (res == EXIT_SUCCESS)
		{
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
	return -5.0f;
}

float ExportSubscribedDataref(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	if (callbackManager->GetSubscribedDatarefCount() < 1)
	{
		json jdataOut = {
		{"Operation", "Empty"}};
		BroadCastData(jdataOut.dump());
		return 1.0f;
	}
	auto* p_subscribedDatarefMap = callbackManager->GetSubscribedDataref();
	json jdataOut = {
		{"Operation", "ExportDataref"},
		{"Datarefs", json::array()}
	};
	std::map<std::string, Dataref*> subDatarefMap = *(callbackManager->GetSubscribedDataref());
	for (auto &kv : *p_subscribedDatarefMap)
	{
		json jdataref = {
			{"Name", kv.first},
			{"Value", kv.second->GetValue()}
		};
		jdataOut["Datarefs"].push_back(jdataref);
	}
	BroadCastData(jdataOut.dump());
	return 0.25f;
}