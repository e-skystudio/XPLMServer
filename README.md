# XPLMServer
X-Plane Server for multi usage purpose

# Compatibility with addons:
- Any addons using the X-Plane dataref system
- FlightFactor A320 Utltimate throught the FlightFactor API (not using export dataref)

# Default behaviour
Working with UDP on port 50555 (for inbound requests) and 50556 (for outbound messages).
This behavour can be changed throught the pluginConfig.json file to be edited in the ``` [X-Plane root]/Resources/plugins/XPLMServer/ ```

## Default Callbacks Operations
- LoadDLL to load other callbacks. **TEST PENDING**
- RegisterDatarefs to save a datarefs with a unique name (example : ``` sim/flightmodels/position/latitude ``` as latitude). To be used when you need to access those datarefs a lot as it skip ```XPLMFindDataref()```. **TEST PASS SUCESSFULLY**
- UnregisterDatarefs : to remove a dataref from the RegisterDatarefs.
- (Un)SubscribeDataref: Mark a registered dataref to be exported every 0.25s. SubscribeDataref: **TEST PASS SUCESSFULLY** | UnSubscribeDataref:  **TEST PENDING**
- GetDatarefValue / SetDatarefValue: Get/Set the value of a dataref (only with non-registered dataref).  **TEST PASS SUCESSFULLY**
- GetRegisteredDatarefValue / SetRegisteredDatarefValue: Get/Set the value of a registered dataref  **TEST PASS SUCESSFULLY**
- Speak : Speak a message in X-Plane  **TEST PASS SUCESSFULLY**
- LoadRegisterDataref: Load a CSV file with dataref to be registered and add them to the subscribe dataref callback.  **TEST PENDING**
- AddConstantDataref : Set a value to a dataref to be updated at every frame with that value **TEST PENDING**
### FlightFactor A320 Specific:
- InitFlightFactorA320 : Initalise the FFA320 API Interface  **TEST PASS SUCESSFULLY**
- RegisterFFDataref : Register a FlightFactor A320 Dataref  **TEST PASS SUCESSFULLY**

# Xplane SDK for compilation
In order to allow older version of xplane to use XPLMServer the SDK folder is adapted.

## On Windows :
Have a environement variable called "xplaneSDK" pointing to the root of your SDK.
In the CHeaders file add a win64 (and/or win32) folder containing the header for the SDK. 
```
SDK\
|- CHeaders/
|     |-  *win32/ (contains the CHeaders of XPlane SDK v2.1.3 or less)
|     |- win64/ (contains the CHeaders of Xplane SDK v3.0.0+ (FOR XP11) or v2.1.3 (or less) (for any other XP version)
|- Libraries/
      |- Mac/
      |- Win/
          |- *XPLM.lib
          |- XPWidgets.lib
          |- *XPLM_64.lib
          |- XPWidgets_64.lib
* = if needed to target 32bits versions
```
## On MacOs:
 - Have one variable environement pointing toward X-Plane name "*xplane*".
 - Have one variable environement pointing toward X-Plane SDK name "*xplane_sdk*"
 - run ```make``` to build the plugin into ```./build/XPLMServer```
 - run ```make publish``` to export toward ```[X-Plane Root]/Resources/plugins/XPLMServer```
