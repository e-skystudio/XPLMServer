#pragma once
#include <functional>
#include <string>
#include <map>

#include <nlohmann/json.hpp>

#include "Dataref.h"

using json = nlohmann::json;


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
	///  Append callback to the stored callback
	/// </summary>
	/// <param name="name">The name of the callback</param>
	/// <param name="newCallback">Function pointer to the callback</param>
	/// <returns>EXIT_SUCESS if the name was not already in use and callback addition was sucessfull</returns>
	int AppendCallback(std::string name, std::function<int(json, CallbackManager*)>* newCallback);
	/// <summary>
	///  Append callback to the stored callback
	/// </summary>
	/// <param name="operation"> The name of the callback</param>
	/// <param name="jsonData"> The json object and argument to be passed to the function</param>
	/// <returns>EXIT_SUCESS if the execution was sucessful</returns>
	int ExecuteCallback(std::string operation, json jsonData);
	/// <summary>
	///  Return the full map of stored named Dataref(s)
	/// </summary>
	/// <returns>
	///  A pointer toward the list of stored datarefs
	/// CAN BE NULL !
	/// </returns>
	std::map<std::string, Dataref*>* GetNamedDataref() const;
	/// <summary>
	///  Return the full map of Subscribed Dataref(s).
	/// </summary>
	/// <returns>
	///  A pointer toward the list of subscribed datarefs.
	/// CAN BE NULL !
	/// </returns>
	std::map<std::string, Dataref*>* GetSubscribedDataref() const;

protected:
	std::map<std::string, std::function<int(json, CallbackManager*)>*>* m_callbacks;
	std::map<std::string, Dataref*>* m_namedDatarefs; //The datarefs stored while plugin is in used
	std::map<std::string, Dataref*>* m_subscribedDatarefs; //The datarefs that value is returned per timed basis
};

///<summary>
/// Define a standard callback function
///</summary>
typedef std::function<int(json, CallbackManager*)> Callback;