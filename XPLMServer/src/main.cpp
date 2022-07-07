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


struct BeaconStatus {
	bool BeaconEnabled = false;
	unsigned int BeaconPort = 0;
};

using json = nlohmann::json;

static json PluginConfiguration;
static CallbackManager* callbackManager;
static int counter = 0;
static UDPServer* server;
static Logger logger;
static std::vector<Client> clients;
static int XplaneVersion;
static int XplaneSDKVersion;
static std::string AircraftICAO;
static std::string AircraftAuthor;
static std::string AircraftDesciption;
static BeaconStatus beaconSts;

float InitializerCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float NetworkCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float ExportSubscribedDataref(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float BeaconCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);

void BroadCastData(std::string data)
{
	for (const Client &c : clients)
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
	std::string name("XPLMServer - DEFAULT");
	std::string description("X-Plane Manipulation Server");
	std::string author("DESSAR Romain");

	if (!PluginConfiguration.contains("Plugin") || !PluginConfiguration.contains("DLLFiles") || !PluginConfiguration.contains("Server"))
	{
		XPLMDebugString("[XPLMServer]Warning Missing Section in config File\n");
		XPLMDebugString("[XPLMServer]Config file should contains : 'Plugin' & 'DLLFiles' & 'Server' Sections\n");
		return 0;
	}
	if(PluginConfiguration["Plugin"].contains("Name")) name = PluginConfiguration["Plugin"]["Name"].get<std::string>();
	if (PluginConfiguration["Plugin"].contains("Description")) description = PluginConfiguration["Plugin"]["Description"].get<std::string>() ;
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

PLUGIN_API int XPluginEnable(void)
{ 
	logger.Log("[XPLMServer]Enabled");
	std::string configuration("");
	std::string platform("");
	#ifndef _DEBUG
		configuration = "Release";
	#else
		configuration = "Debug";
	#endif
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
	XPLMDebugString("[0]\n");
	logger.Log("Loading configuration :'" + configuration + "' & platform : '" + platform + "'");
	if (!PluginConfiguration["DLLFiles"].contains(platform))
	{
		XPLMDebugString(std::string("[XPLMServer]Warning Missing Config Section in ['DLLFiles']['" + platform + "']\n").c_str());
		return 0;
	}
	if (!PluginConfiguration["DLLFiles"][platform].contains(configuration))
	{
		XPLMDebugString(std::string("[XPLMServer]Warning Missing Config Section in ['DLLFiles']['" + platform + "']['" + configuration + "']\n").c_str());
		return 0;
	}
	std::string dllPath = PluginConfiguration["DLLFiles"][platform][configuration].get<std::string>();
	callbackManager = new CallbackManager();
	int res = callbackManager->LoadCallbackDLL(dllPath);
	std::stringstream debug;
	logger.Log(debug.str());
	logger.Log("---Server Init----");
	unsigned short portIn(50556);
	if (PluginConfiguration["Server"].contains("InPort"))
	{
		portIn = PluginConfiguration["Server"]["InPort"].get<unsigned short>();
	}
	else {
		logger.Log("[XPLMServer]Missing Config['Server']['InPort']... defaulting to 50556\n");
	}
	unsigned short portOut(50555);
	if (PluginConfiguration["Server"].contains("OutPort"))
	{
		portIn = PluginConfiguration["Server"]["OutPort"].get<unsigned short>();
	}
	else {
		logger.Log("[XPLMServer]Missing Config['Server']['OutPort']... defaulting to 50555\n");
	}
	XPLMDebugString("[4]\n");
	if (PluginConfiguration.contains("Server"))
	{
		if (PluginConfiguration["Server"].contains("BaconEnabled") && PluginConfiguration["Server"].contains("BeaconPort"))
		{
			beaconSts.BeaconEnabled = PluginConfiguration["Server"]["BaconEnabled"].get<bool>();
			beaconSts.BeaconPort = PluginConfiguration["Server"]["BeaconPort"].get<unsigned int>();
		}
		if (PluginConfiguration["Server"].contains("InIp") &&
			PluginConfiguration["Server"].contains("InPort"))
		{
			server = new UDPServer();
			res = server->Bind(portIn, portOut, beaconSts.BeaconEnabled);
			if (res == EXIT_SUCCESS)
			{
				XPLMRegisterFlightLoopCallback(InitializerCallback, -1.0f, nullptr);
			}
			else
			{
				logger.Log("Initalization failed, Res was " + std::to_string(res));
			}
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
	/*bool beaconEnabled = false;
	if (PluginConfiguration["Server"].contains("BeaconEnabled")) {
		beaconEnabled = PluginConfiguration["Server"]["BeaconEnabled"].get<bool>();
	}*/
	if(beaconSts.BeaconEnabled)
	{
		XPLMRegisterFlightLoopCallback(BeaconCallback, -1.0f, nullptr);
	}
 	return 0.0f;
}

float NetworkCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	if(!callbackManager->IsFF320InterfaceEnabled())
	{
		callbackManager->ExecuteConstantDataref();
	}
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
	BroadCastData(jdataOut.dump());
	return 0.25f;
}

float BeaconCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	json jdataOut = {
		{"Operation", "Beacon"},
		{"XplaneVersion", XplaneVersion},
		{"XplaneSDKVersion", XplaneSDKVersion},
		{"AircraftICAO", AircraftICAO},
		{"AircraftDescription", AircraftDesciption},
		{"AircraftAuthor", AircraftAuthor},
		{"AircraftAuthor", AircraftAuthor},
		{"XplaneUDPReceive", server->GetInboundPort()},
		{"XplaneUDPEmit", server->GetOutboundPort()},
	};
	BroadCastData(jdataOut.dump());
	int beaconPort = PluginConfiguration["Server"]["BeaconPort"].get<int>();
	server->BroadcastData(jdataOut.dump(), beaconSts.BeaconPort);
	return 1.0f;
}