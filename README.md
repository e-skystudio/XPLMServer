# XPLMServer
X-Plane Server for multi usage purpose

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
