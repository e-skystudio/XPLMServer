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

/////<summary>
///// Default callback struct.
/////</summary>
//struct CallbackFunction {
//	const char* operation; ///The json Operation value to execute the callback
//	const char* function; /// The name of the function in the DLL.
//};
//
//typedef int(*callback)(json*, CallbackManager*); ///The callback reference

constexpr int CallbackNumber = 2;

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
CALLBACK_FUNC void GetCallbacks(CallbackFunction** callbacks, int* size);

CALLBACK_FUNC int SetVisibility(json jdata, CallbackManager* callbackManager);
///<summary>
/// Default callback to load a DLL.
/// This callback should be used carefully as DLLs shall not be thursted.
///</summary>
CALLBACK_FUNC int LoadDll(json jdata, CallbackManager* callbackManager);