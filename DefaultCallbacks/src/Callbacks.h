#pragma once
#include "CallbackManager.h"
#include "Dataref.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#ifdef MAKE_DLL
#define CALLBACK_FUNC extern "C" __declspec(dllexport) 
#else
#define CALLBACK_FUNC extern "C" __declspec(dllimport) 
#endif

constexpr int CallbackNumber = 10;

///<summary>
/// Manadatory function to implement.
/// Caution when callbacks is NULL, the function should only return the number of callback
/// that will be returned.
///</summary>
///<param name="callbacks">Pointer to an array of callbacks (can be nullptr/NULL)</params>
///<param name="size">
/// Pointer to an int, will be set as the number of callback
/// return(able) by the function
///</params>
CALLBACK_FUNC void GetCallbacks(std::vector<CallbackFunction*>* callbacks, int* size);


///<summary>
/// Debug callback to set visibility to the value passed.
///</summary>
CALLBACK_FUNC int SetVisibility(json* jdata, CallbackManager* callbackManager);
///<summary>
/// Default callback to load a DLL.
/// This callback should be used carefully as DLLs shall not be thursted.
///</summary>
CALLBACK_FUNC int LoadDll(json* jdata, CallbackManager* callbackManager);

///<summary>
/// Add a dataref to the list of registered dataref.
/// JSON Should contains:
///    - Name: the name of the dataref
///    - Link: the link to the dataref
///    - *Type: the type of the dataref (if not set, will be loaded from SDK)
///    - *ConversionFactor: the conversion factor of the dataref (if not set, will be set to 1.0)
///</summary>
CALLBACK_FUNC int RegisterDataref(json* jdata, CallbackManager* callbackManager);

///<summary>
/// Remove a dataref to the list of registered dataref.
/// JSON Should contains:
///    - Name: the name of the dataref
///</summary>
CALLBACK_FUNC int UnregisterDataref(json* jdata, CallbackManager* callbackManager);

///<summary>
/// Subscribe to the dataref: auto send his value every 0.5s
/// If dataref is already register JSON Should contains:
///    - Name: the name of the dataref to be looked up into the Registered Datarefs
/// Else:
///	   - Name: the name of the dataref
///    - Link: the link to the dataref
///    - *Type : the type of the dataref(if not set, will be loaded from SDK)
///    - *ConversionFactor : the conversion factor of the dataref(if not set, will be set to 1.0)
///</summary>
CALLBACK_FUNC int SubscribeDataref(json* jdata, CallbackManager* callbackManager);

///<summary>
/// Remove a dataref from the subscribe dataref.
/// !This will not remove the dataref from the registered datarefs.!
/// JSON Should contains:
///    - Name: the name of the dataref
///</summary>
CALLBACK_FUNC int UnsubscribeDataref(json* jdata, CallbackManager* callbackManager);

///<summary>
/// Return the value of the registered dataref.
/// JSON Should contains:
///    - Name: the name of the dataref
///</summary>
CALLBACK_FUNC int GetRegisterDatarefValue(json* jdata, CallbackManager* callbackManager);

///<summary>
/// Set the value of the register dataref.
/// JSON Should contains:
///    - Name: the name of the dataref
///    - Value: the value of the dataref
///</summary>
CALLBACK_FUNC int SetRegisterDatarefValue(json* jdata, CallbackManager* callbackManager);

///<summary>
/// Return the value of a dataref. This callback will take longer to execute than
/// if the dataref is registered, if a dataref shall be accessed a lot, consider registering it.
/// JSON Should contains:
///    - Name: the name of the dataref
///</summary>
CALLBACK_FUNC int GetDatarefValue(json* jdata, CallbackManager* callbackManager);

///<summary>
/// Set the valueof a dataref. This callback will take longer to execute than
/// if the dataref is registered, if a dataref shall be accessed a lot, consider registering it.
/// JSON Should contains:
///    - Name: the name of the dataref
///    - Value: the value of the dataref
///</summary>
CALLBACK_FUNC int SetDatarefValue(json* jdata, CallbackManager* callbackManager);