// ReSharper disable CppClangTidyMiscMisplacedConst

#pragma warning(disable : 6054)

#include "UDPServer.h" //avoid error on windows
#include "Beacon.h"

#include <sstream>
#include <string>

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#include <Dataref.h>
#include <CallbackManager.h>
#include <utils.h>
#include <nlohmann/json.hpp>


struct BeaconStatus {
	bool BeaconEnabled = false;
	unsigned short BeaconPort = 0;
};

using json = nlohmann::json;

static json PLUGIN_CONFIGURATION;
static CallbackManager* CALLBACK_MANAGER;
static UdpServer* SERVER;
static Logger LOGGER;
static std::vector<Client> CLIENTS;
static int XPLANE_VERSION;
static int XPLANE_SDK_VERSION;
static std::string AIRCRAFT_ICAO;
static std::string AIRCRAFT_AUTHOR;
static std::string AIRCRAFT_DESCIPTION;
static bool BEACON_ENABLED = false;
static Beacon* BEACON;

static XPLMDataRef acft_author = XPLMFindDataRef("sim/aircraft/view/acf_author");
static XPLMDataRef acft_description = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
static XPLMDataRef acft_icao = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");

float InitializerCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float NetworkCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float ExportSubscribedDataref(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
float BeaconCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef);
void ConfigureBeacon();

void BroadCastData(const std::string& data)
{
	for (const Client &c : CLIENTS)
	{
		int _ = SERVER->SendData(data, c);
	}
}

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
	LOGGER = Logger("XPLMServer.log", "[XPLMServer]", true);
	#ifdef IBM
	auto data = loadFile(R"(.\Resources\plugins\XPLMServer\pluginConfig.json)");
	#else
	auto data = loadFile("./Resources/plugins/XPLMServer/pluginConfig.json");
	#endif
	if (data.str().length() < 1)
	{
		XPLMDebugString("[XPLMServer]Unable to load configuration file\n");
		return 0;
	}
	PLUGIN_CONFIGURATION = json::parse(data.str());
	std::string name("XPLMServer - DEFAULT");
	std::string description("X-Plane Manipulation Server");
	std::string author("DESSAR Romain");

	if (!PLUGIN_CONFIGURATION.contains("Plugin") || !PLUGIN_CONFIGURATION.contains("DLLFiles") || !PLUGIN_CONFIGURATION.contains("Server"))
	{
		XPLMDebugString("[XPLMServer]Warning Missing Section in config File\n");
		XPLMDebugString("[XPLMServer]Config file should contains : 'Plugin' & 'DLLFiles' & 'Server' Sections\n");
		return 0;
	}
	if(PLUGIN_CONFIGURATION["Plugin"].contains("Name")) name = PLUGIN_CONFIGURATION["Plugin"]["Name"].get<std::string>();
	if (PLUGIN_CONFIGURATION["Plugin"].contains("Description")) description = PLUGIN_CONFIGURATION["Plugin"]["Description"].get<std::string>() ;
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
	XPLMGetVersions(&XPLANE_VERSION, &XPLANE_SDK_VERSION, &hostId);
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
	LOGGER.Log("[XPLMServer]Enabled");
	std::string configuration;
	std::string platform;
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
	LOGGER.Log("Loading configuration :'" + configuration + "' & platform : '" + platform + "'");
	if (!PLUGIN_CONFIGURATION["DLLFiles"].contains(platform))
	{
		XPLMDebugString(std::string("[XPLMServer]Warning Missing Config Section in ['DLLFiles']['" + platform + "']\n").c_str());
		return 0;
	}
	if (!PLUGIN_CONFIGURATION["DLLFiles"][platform].contains(configuration))
	{
		XPLMDebugString(std::string("[XPLMServer]Warning Missing Config Section in ['DLLFiles']['" + platform + "']['" + configuration + "']\n").c_str());
		return 0;
	}
	const std::string dllPath = PLUGIN_CONFIGURATION["DLLFiles"][platform][configuration].get<std::string>();
	CALLBACK_MANAGER = new CallbackManager();
	int res = CALLBACK_MANAGER->LoadCallbackDLL(dllPath);
	LOGGER.Log("---Server Init----");
	unsigned short portIn(50555);
	unsigned short portOut(50556);

	if (PLUGIN_CONFIGURATION["Server"].contains("InPort"))
	{
		portIn = PLUGIN_CONFIGURATION["Server"]["InPort"].get<unsigned short>();
	}
	else {
		LOGGER.Log("[XPLMServer]Missing Config['Server']['InPort']... defaulting to 50556\n");
	}
	if (PLUGIN_CONFIGURATION["Server"].contains("OutPort"))
	{
		portOut = PLUGIN_CONFIGURATION["Server"]["OutPort"].get<unsigned short>();
	}
	else {
		LOGGER.Log("[XPLMServer]Missing Config['Server']['OutPort']... defaulting to 50555\n");
	}
	if (PLUGIN_CONFIGURATION.contains("Server") && !PLUGIN_CONFIGURATION["Server"].is_null())
	{
		ConfigureBeacon();
		SERVER = new UdpServer();
		if (const int serverInitRes = SERVER->Bind(portIn, portOut) != EXIT_SUCCESS)
		{
			LOGGER.Log("Initalization failed, Res was " + std::to_string(serverInitRes));
			return 0;
		}
		XPLMRegisterFlightLoopCallback(InitializerCallback, -1.0f, nullptr);
	}
	acft_author = XPLMFindDataRef("sim/aircraft/view/acf_author");
	acft_description = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
	acft_icao = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
	return 1;
}

// ReSharper disable once CppParameterMayBeConst
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) 
{
	if (CALLBACK_MANAGER->GetSubscribedEventMap()->contains(static_cast<unsigned>(inMsg)))
	{
		json ops;
		ops["Operation"] = "Event Triggered";
		ops["Value"] = CALLBACK_MANAGER->GetSubscribedEventMap()->at(static_cast<unsigned>(inMsg));
		BroadCastData(ops.dump());
	}
}

void ConfigureBeacon()
{
	if (PLUGIN_CONFIGURATION["Server"].contains("Beacon") && !PLUGIN_CONFIGURATION["Server"]["Beacon"].is_null())
	{
		json beaconConfig = PLUGIN_CONFIGURATION["Server"]["Beacon"];
		if (beaconConfig.contains("Enabled") && !beaconConfig["Enabled"].is_null() && beaconConfig["Enabled"].get<bool>())
		{
			if (!beaconConfig.contains("Port") || beaconConfig["Port"].is_null()) return;
			const auto beaconPort = beaconConfig["Port"].get<unsigned short>();
			if (beaconConfig.contains("Broadcast") && !beaconConfig["Broadcast"].is_null())
			{
				BEACON = new Beacon();
				if (BEACON->Configure("", beaconPort, true) == 0) BEACON_ENABLED = true;
				return;
			}
			if (!beaconConfig.contains("Ip") || beaconConfig["Ip"].is_null()) return;
			const auto beaconIp = beaconConfig["Ip"].get<std::string>();
			BEACON = new Beacon();
			if (BEACON->Configure(beaconIp, beaconPort, false) == 0) BEACON_ENABLED = true;
		}
	}
}

// ReSharper disable once CppParameterNeverUsed
float InitializerCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{

	XPLMRegisterFlightLoopCallback(NetworkCallback, -1.0f, nullptr);
	XPLMRegisterFlightLoopCallback(ExportSubscribedDataref, -1.0f, nullptr);
	if(BEACON_ENABLED)
	{
		XPLMRegisterFlightLoopCallback(BeaconCallback, -1.0f, nullptr);
	}
 	return 0.0f;
}

// ReSharper disable once CppParameterNeverUsed
float NetworkCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	if(!CALLBACK_MANAGER->IsFF320InterfaceEnabled())
	{
		CALLBACK_MANAGER->ExecuteConstantDataref();
	}
	Client cli;
	std::string data = SERVER->ReceiveData(4096, &cli);
	if (data.length() < 1)
		return -1.0f;
	json operation = json::parse(data);
	CALLBACK_MANAGER->ExecuteCallback(&operation);
	BroadCastData(operation.dump());
	
	bool foundClient = false;
	for (const auto& [ip, port] : CLIENTS)
	{
		if (ip == cli.Ip && port == cli.Port)
		{
			foundClient = true;
			break;
		}
	}

	if (!foundClient)
	{
		CLIENTS.push_back(cli);
	}
	return -1.0f;
}

// ReSharper disable once CppParameterNeverUsed
float ExportSubscribedDataref(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
	const auto* p_subscribedDatarefMap = CALLBACK_MANAGER->GetSubscribedDataref();
	json jdataOut = {
		{"Operation", "ExportDataref"},
		{"Datarefs", json::array()}
	};
	std::map<std::string, AbstractDataref*> subDatarefMap = *(CALLBACK_MANAGER->GetSubscribedDataref());
	for (const auto & [fst, snd] : *p_subscribedDatarefMap)
	{
		json jdataref = {
			{"Name", fst},
			{"Value", snd->GetValue()}
		};
		jdataOut["Datarefs"].push_back(jdataref);
	}
	BroadCastData(jdataOut.dump());
	return 0.25f;
}

// ReSharper disable once CppParameterNeverUsed
float BeaconCallback(float elapsedSinceCall, float elapsedSinceLastTime, int inCounter, void* inRef)
{
#pragma region GettingInfoAboutLoadedAircraft
	char author[500];
	char description[500];
	char icao[40];
	std::string aircraft_author, aircraft_description;

	int size = XPLMGetDatab(acft_author, (void*)author, 0, 500);
	aircraft_author = std::string(author).substr(0, size),  // NOLINT(clang-diagnostic-comma)
	size = XPLMGetDatab(acft_description, (void*)description, 0, 500);
	aircraft_description = std::string(description).substr(0, size),
	size = XPLMGetDatab(acft_icao, (void*)icao, 0, 40);
	std::string aircraft_icao = std::string(icao).substr(0, size);
#pragma endregion
	json jdataOut = {
		{"Operation", "Beacon"},
		{"Simulator", "XPLANE"},
		{"SimulatorVersion", XPLANE_VERSION},
		{"SimulatorSDKVersion", XPLANE_SDK_VERSION},
		{"SimulatorProtocol", "UDP"},
		{"SimulatorReceive", SERVER->GetInboundPort()},
		{"SimulatorEmit", SERVER->GetOutboundPort()},
		{"SimulatorIp", SERVER->GetLocalIp()},
		{"AircraftAuthor", aircraft_author},
		{"AircraftDescription", aircraft_description},
		{"AircraftICAO", aircraft_icao},
	};
	BroadCastData(jdataOut.dump());
	int _ = BEACON->SendData(jdataOut.dump());
	return 1.0f;
}
