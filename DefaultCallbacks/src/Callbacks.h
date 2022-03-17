#pragma once
#include "CallbackManager.h"
#include "Dataref.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
#ifdef IBM
    #ifdef MAKE_DLL
    #define CALLBACK_FUNC extern "C" __declspec(dllexport) 
    #else
    #define CALLBACK_FUNC extern "C" __declspec(dllimport) 
    #endif
#else 
    #define CALLBACK_FUNC extern "C" __attribute__((visibility("default")))
#endif

constexpr int CallbackNumber = 12;

std::string ExtractJsonValue(json* jdata, std::string fieldname, CallbackManager* callback);

// 1
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
CALLBACK_FUNC void GetCallbacks(std::vector<CallbackFunctionStruct*>* callbacks, int* size);

// 2
///<summary>
/// Default callback to load a DLL.
/// This callback should be used carefully as DLLs shall not be thursted.
///</summary>
CALLBACK_FUNC int LoadDll(json* jdata, CallbackManager* callbackManager);

// 3
///<summary>
/// Add a dataref to the list of registered dataref.
/// JSON Should contains:
///    - Name: the name of the dataref
///    - Link: the link to the dataref
///    - *Type: the type of the dataref (if not set, will be loaded from SDK)
///    - *ConversionFactor: the conversion factor of the dataref (if not set, will be set to 1.0)
///</summary>
CALLBACK_FUNC int RegisterDataref(json* jdata, CallbackManager* callbackManager);

// 4
///<summary>
/// Remove a dataref to the list of registered dataref.
/// JSON Should contains:
///    - Name: the name of the dataref
///</summary>
CALLBACK_FUNC int UnregisterDataref(json* jdata, CallbackManager* callbackManager);

// 5
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

// 6
///<summary>
/// Remove a dataref from the subscribe dataref.
/// !This will not remove the dataref from the registered datarefs.!
/// JSON Should contains:
///    - Name: the name of the dataref
///</summary>
CALLBACK_FUNC int UnsubscribeDataref(json* jdata, CallbackManager* callbackManager);

// 7
///<summary>
/// Return the value of the registered dataref.
/// JSON Should contains:
///    - Name: the name of the dataref
///</summary>
CALLBACK_FUNC int GetRegisterDatarefValue(json* jdata, CallbackManager* callbackManager);

// 8
///<summary>
/// Set the value of the register dataref.
/// JSON Should contains:
///    - Name: the name of the dataref
///    - Value: the value of the dataref
///</summary>
CALLBACK_FUNC int SetRegisterDatarefValue(json* jdata, CallbackManager* callbackManager);

// 9
///<summary>
/// Return the value of a dataref. This callback will take longer to execute than
/// if the dataref is registered, if a dataref shall be accessed a lot, consider registering it.
/// JSON Should contains:
///    - Name: the name of the dataref
///</summary>
CALLBACK_FUNC int GetDatarefValue(json* jdata, CallbackManager* callbackManager);

// 10
///<summary>
/// Set the valueof a dataref. This callback will take longer to execute than
/// if the dataref is registered, if a dataref shall be accessed a lot, consider registering it.
/// JSON Should contains:
///    - Name: the name of the dataref
///    - Value: the value of the dataref
///</summary>
CALLBACK_FUNC int SetDatarefValue(json* jdata, CallbackManager* callbackManager);

// 11
///<summary>
/// Using X-Plane Text to speach to say a phrase in Xplane.
/// JSON Should contains:
///    - Text: the text to be spoken
///</summary>
CALLBACK_FUNC int Speak(json* jdata, CallbackManager* callback);

// 12
///<summary>
/// Load a series of dataref to be exported; file should be .csv
/// JSON Should contains:
///    - FileIn: the path (relative to xplane.exe) of the dataref to be parsed.
///</summary>
CALLBACK_FUNC int AddConstantDataref(json* jdata, CallbackManager* callback);

//14
CALLBACK_FUNC int LoadRegisterDataref(json* jdata, CallbackManager* callback);