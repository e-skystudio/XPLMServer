#pragma once


#ifdef DLL_MAKE
#define CALLBACK extern "C" __declspec(dllexport) 
#else
#define CALLBACK extern "C" __declspec(dllimport) 
#endif


///<summary>
/// Default callback struct.
///</summary>
struct CallbackFunction {
	const char* operation; ///The json Operation value to execute the callback
	const char* function; /// The name of the function in the DLL.
};

typedef void(*callbackLoader)(CallbackFunction**, int*);
typedef int(*callback)(const char* json, void* CallbackManager); ///The callback reference

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
CALLBACK void GetCallbacks(CallbackFunction** callbacks, int* size);