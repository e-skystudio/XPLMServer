// ReSharper disable CppInconsistentNaming
// ReSharper disable CppClangTidyModernizeUseNodiscard
#pragma once
#ifdef IBM
	#include <Windows.h>
#else
	#include <dlfcn.h>
#endif

#include <functional>
#include <string>
#include <map>
#include <sstream>
#include <queue>

#include <nlohmann/json.hpp>
#include <utility>

#include "Dataref.h"
#include "FFDataref.h"
#include "utils.h"
#include "SharedValue.h"

#include <XPLMPlugin.h>

using json = nlohmann::json;

#pragma region DLLManagement
struct CallbackFunctionStruct {
	std::string Operation; ///The json Operation value to execute the callback
	std::string Function; /// The name of the function in the DLL.
	
	CallbackFunctionStruct();

	CallbackFunctionStruct(std::string operation, std::string functionName);
};

typedef void(*CallbackLoader)(std::vector<CallbackFunctionStruct*>*, int*);
typedef int(*Callback)(json* json, void* CallbackManager); ///The callback reference

struct ConstantDataref {
	std::string Name;
	std::string Value;
	AbstractDataref* Dataref{};
};
#pragma endregion

void FF320_Callback(double step, void* tag);

///<summary>
/// Provide an OOP Handle to callbacks
///</summary>
class CallbackManager
{
public:
	///<summary>
	/// Default Constructor
	///</summary>
	CallbackManager();
	///<summary>
	/// Copy Constructor
	///</summary>
	CallbackManager(const CallbackManager& rhs);
	/// <summary>
	/// Default destructor
	/// </summary>
	~CallbackManager();
	/// <summary>
	///  Append callback to the stored callback
	/// </summary>
	/// <param name="name">The name of the callback</param>
	/// <param name="newCallback">Function pointer to the callback</param>
	/// <returns>EXIT_SUCESS if the name was not already in use and callback addition was sucessfull</returns>
	int AppendCallback(const std::string& name, Callback newCallback) const;
	/// <summary>
	///  Append callback to the stored callback
	/// </summary>
	/// <param name="operation"> The name of the callback</param>
	/// <param name="jsonData"> The json object and argument to be passed to the function</param>
	/// <returns>EXIT_SUCESS if the execution was sucessful</returns>
	int ExecuteCallback(json* jsonData);
	/// <summary>
	///  Return the full map of stored named Dataref(s)
	/// </summary>
	/// <returns>
	///  A pointer toward the list of stored datarefs
	/// CAN BE NULL !
	/// </returns>
	
	std::map<std::string, AbstractDataref*>* GetNamedDataref() const;
	/// <summary>
	///  Return the full map of Subscribed Dataref(s).
	/// </summary>
	/// <returns>
	///  A pointer toward the list of subscribed datarefs.
	/// CAN BE NULL !
	/// </returns>
	std::map<std::string, AbstractDataref*>* GetSubscribedDataref() const;
	/// <summary>
	///  Return the full map of Subscribed Event(s).
	/// </summary>
	/// <returns>
	///  A pointer toward the list of subscribed events.
	/// CAN BE NULL !
	/// </returns>
	std::map<unsigned int, std::string>* GetSubscribedEventMap() const;
	/// <summary>
	/// Load callbacks from DLL file.
	/// DLL Must implement the GetCallbacks();
	/// </summary>
	/// <param name="inDllPath">The path to the DLL</param>
	/// <returns>
	///	An integer with the number of callback added.
	/// If it's negative then it report an error
	/// </returns
	int LoadCallbackDLL(const std::string& inDllPath);
	/// <summary>
	///	Access the underlying logger, to be used only by DLLs callback
	/// wanted to make an entry to the logger.
	/// </summary>
	/// <param name="data">The data to log</param>
	/// <param name="severity">The severity of the log DEFAULT: Debug</param>
	void Log(const std::string& data, Logger::Severity severity = Logger::Severity::DEBUG);
	/// <summary>
	/// Return the number of subcibded Datarefs
	/// </summary>
	int GetSubscribedDatarefCount() const;
	void AddSubscribedDataref(const std::string& name);
	void RemoveSubscribedDataref(const std::string& name);
	int GetConstantDatarefCount() const;
	void AddConstantDataref(const std::string& name, const std::string& value);
	void RemoveConstantDataref(const std::string& name) const;
	void ExecuteConstantDataref() const;
	int ExecuteFFDatarefsUpdate();
	void AddFFDatarefToUpdate(FFDataref* dataref);
	SharedValuesInterface* GetFF320Interface() const;
	bool InitFF320Interface();
	bool IsFF320InterfaceEnabled() const;
	// void BindFF320Callback(SharedDataUpdateProc callback);
protected:
	std::map<std::string, Callback>* m_callbacks;
	std::map<std::string, AbstractDataref*>* m_namedDatarefs; //The datarefs stored while plugin is in used
	// std::map<std::string, FFDataref*>* m_namedFFDatarefs;
	std::map<std::string, AbstractDataref*>* m_subscribedDatarefs; //The datarefs that value is returned per timed basis
	std::map<std::string, ConstantDataref>* m_constDataref; //Datarefs set as constant (value are copied from the key)
	std::map<unsigned int, std::string>* m_subscribedEvent;
	// std::queue<ConstantDataref>* m_ff320_const_datarefs;
	std::queue<FFDataref*> m_ff320_datarefs;
	Logger m_logger; /* The logger */
	unsigned int m_subscirbeDatarefCount;
	#ifdef IBM
		HINSTANCE m_hDLL;
	#else
		void* m_hDLL;
	#endif
	SharedValuesInterface* m_ff320;
};