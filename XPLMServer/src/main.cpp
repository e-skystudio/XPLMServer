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
static int XplaneVersion;
static int XplaneSDKVersion;
static std::string AircraftICAO;
static std::string AircraftAuthor;
static std::string AircraftDesciption;

void BroadCastData(std::string data)
{
	server->BroadcastData(data);
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
	std::string name = PluginConfiguration["Plugin"]["Name"].get<std::string>();
	std::string description = PluginConfiguration["Plugin"]["Description"].get<std::string>() ;
	std::string signature = "eskystudio.tools.XPLMServer";
#ifdef IBM
	strcpy_s(outName, strlen(name.c_str()) + 1, name.c_str());
	strcpy_s(outSig, strlen(description.c_str()) + 1, description.c_str());
	strcpy_s(outDesc, strlen(signature.c_str()) + 1, signature.c_str());
#else
	strcpy(outName, name.c_str());
	strcpy(outSig, signature.c_str());
	strcpy(outDesc, description.c_str());
#endif
	XPLMHostApplicationID hostId;
	XPLMGetVersions(&XplaneVersion, &XplaneSDKVersion, &hostId);



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
#pragma region GettingInfoAboutLoadedAircraft
	XPLMDataRef acftAuthor = XPLMFindDataRef("sim/aircraft/view/acf_author");
	XPLMDataRef acftDescription = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
	XPLMDataRef acftICAO = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");

	char author[500];
	char description[500];
	char icao[40];

	int size(0);
	size = XPLMGetDatab(acftAuthor, (void*)author, 0, 500);
	AircraftAuthor = std::string(author).substr(0, size),
	size = XPLMGetDatab(acftDescription, (void*)description, 0, 500);
	AircraftDesciption = std::string(description).substr(0, size),
	size = XPLMGetDatab(acftICAO, (void*)icao, 0, 40);
	AircraftICAO = std::string(icao).substr(0, size);
#pragma endregion

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
	json operation = json::parse(data);
	callbackManager->ExecuteCallback(&operation);
	BroadCastData(operation.dump());
	
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
	if (callbackManager->GetSubscribedDatarefCount() <= 0 )
	{
		json jdataOut = {
			{"Operation", "Beacon"},
			{"XplaneVersion", XplaneVersion},
			{"XplaneSDKVersion", XplaneSDKVersion},
			{"AircraftICAO", AircraftICAO},
			{"AircraftDescription", AircraftDesciption},
			{"AircraftAuthor", AircraftAuthor},
		};
		BroadCastData(jdataOut.dump());
		XPLMDebugString("[XPLMServer]Sending Empty ops beacon...\n");
		return 1.0f;
	}
	auto* p_subscribedDatarefMap = callbackManager->GetSubscribedDataref();
	json jdataOut = {
		{"Operation", "ExportDataref"},
		{"Datarefs", json::array()}
	};
	std::map<std::string, AbstractDataref*> subDatarefMap = *(callbackManager->GetSubscribedDataref());
	for (auto &kv : *p_subscribedDatarefMap)
	{
		json jdataref = {
			{"Name", kv.first},
			{"Value", kv.second->GetValue()}
		};
		jdataOut["Datarefs"].push_back(jdataref);
	}
	XPLMDebugString(jdataOut.dump().c_str());
	XPLMDebugString("\n");
	BroadCastData(jdataOut.dump());
	return 0.25f;
}