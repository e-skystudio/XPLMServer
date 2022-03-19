SDK			    := $(xplane_sdk)
XPLANE          := $(xplane)
TARGET		    := XPLMServer
BASE_DIR        := $(PWD)
BUILDDIR	    := $(BASE_DIR)/build
CALLBACKMANAGER := $(BASE_DIR)/CallbackManager
DEFAULTCALLBACK := $(BASE_DIR)/DefaultCallbacks
NETWORKING      := $(BASE_DIR)/Networking
XPLMServer 	    := $(BASE_DIR)/XPLMServer
VCPKG_INCLUDES  := /Users/rdessart/Projects/CPP/vcpkg/installed/arm64-osx/include/
OUTPUT_DIR      := $(BUILDDIR)/XPLMServer

LIB := -F $(SDK)/Libraries/Mac\
       -framework XPLM\
       -framework XPWidgets

INCLUDES = \
	-I$(SDK)/CHeaders/XPLM\
	-I$(CALLBACKMANAGER)/include\
	-I$(NETWORKING)/include\
	-I$(VCPKG_INCLUDES)/

DEFINES = -DXPLM200=1 -DXPLM210=1 -DAPL=1 -D_DEBUG=1 

CFLAGS := $(DEFINES) $(INCLUDES) -std=c++20 -fPIC -fvisibility=hidden -arch x86_64

###XPLMServer
$(BUILDDIR)/mac.xpl : $(BUILDDIR)/UDPServer.o $(BUILDDIR)/XPLMServer.o | $(BUILDDIR) $(BUILDDIR)/libNetworking.a $(BUILDDIR)/libCallbackManager.a $(OUTPUT_DIR)/DefaultCallbacks64.dylib $(OUTPUT_DIR)
	g++ $(CFLAGS) -m64 -dynamiclib -Wl, $(BUILDDIR)/UDPServer.o $(BUILDDIR)/Dataref.o $(BUILDDIR)/Logger.o $(BUILDDIR)/utils.o $(BUILDDIR)/CallbackManager.o $(BUILDDIR)/XPLMServer.o  -o $(OUTPUT_DIR)/64/mac.xpl $(LIB)

$(BUILDDIR)/XPLMServer.o : $(XPLMServer)/src/main.cpp
	g++ $(CFLAGS) $(XPLMServer)/src/main.cpp -c -o $(BUILDDIR)/XPLMServer.o

### DEFAULT CALLBACK

$(OUTPUT_DIR)/DefaultCallbacks64.dylib : $(BUILDDIR)/DefaultCallbacks.o | $(BUILDDIR)/libCallbackManager.a $(OUTPUT_DIR)
	g++ $(CFLAGS) -fvisibility=default -dynamiclib -Wl, -o $(OUTPUT_DIR)/DefaultCallbacks64.dylib $(BUILDDIR)/DefaultCallbacks.o $(LIB) -lCallbackManager -L$(BUILDDIR)

$(BUILDDIR)/DefaultCallbacks.o : $(DEFAULTCALLBACK)/src/Callbacks.cpp $(DEFAULTCALLBACK)/src/Callbacks.h | $(BUILDDIR)
	g++ $(CFLAGS) $(DEFAULTCALLBACK)/src/Callbacks.cpp -c -o $(BUILDDIR)/DefaultCallbacks.o

### Networking
$(BUILDDIR)/libNetworking.a: $(BUILDDIR)/UDPServer.o | $(BUILDDIR)
	ar rcs $(BUILDDIR)/libNetworking.a $(BUILDDIR)/UDPServer.o

$(BUILDDIR)/UDPServer.o : $(NETWORKING)/src/UDPServer.cpp | $(BUILDDIR)
	g++ $(CFLAGS) $(NETWORKING)/src/UDPServer.cpp -c -o $(BUILDDIR)/UDPServer.o

### CALLBACKMANAGER
$(BUILDDIR)/libCallbackManager.a: $(BUILDDIR)/Dataref.o $(BUILDDIR)/Logger.o $(BUILDDIR)/utils.o $(BUILDDIR)/CallbackManager.o $(BUILDDIR)/FFDataref.o | $(BUILDDIR)
	ar rcs $(BUILDDIR)/libCallbackManager.a $(BUILDDIR)/Dataref.o $(BUILDDIR)/Logger.o $(BUILDDIR)/utils.o $(BUILDDIR)/CallbackManager.o $(BUILDDIR)/FFDataref.o

$(BUILDDIR)/Dataref.o : $(CALLBACKMANAGER)/src/Dataref.cpp $(CALLBACKMANAGER)/include/Dataref.h | $(BUILDDIR)
	g++ $(CFLAGS) $(CALLBACKMANAGER)/src/Dataref.cpp -c -o $(BUILDDIR)/Dataref.o

$(BUILDDIR)/FFDataref.o : $(CALLBACKMANAGER)/src/FFDataref.cpp $(CALLBACKMANAGER)/include/FFDataref.h | $(BUILDDIR)
	g++ $(CFLAGS) $(CALLBACKMANAGER)/src/FFDataref.cpp -c -o $(BUILDDIR)/FFDataref.o

$(BUILDDIR)/Logger.o : $(CALLBACKMANAGER)/src/Logger.cpp $(CALLBACKMANAGER)/include/Logger.h | $(BUILDDIR)
	g++ $(CFLAGS) $(CALLBACKMANAGER)/src/Logger.cpp -c -o $(BUILDDIR)/Logger.o

$(BUILDDIR)/utils.o : $(CALLBACKMANAGER)/src/utils.cpp $(CALLBACKMANAGER)/include/utils.h | $(BUILDDIR)
	g++ $(CFLAGS) $(CALLBACKMANAGER)/src/utils.cpp -c -o $(BUILDDIR)/utils.o

$(BUILDDIR)/CallbackManager.o : $(CALLBACKMANAGER)/src/CallbackManager.cpp $(CALLBACKMANAGER)/include/CallbackManager.h $(CALLBACKMANAGER)/include/AbstractDataref.h | $(BUILDDIR)
	g++ $(CFLAGS) $(CALLBACKMANAGER)/src/CallbackManager.cpp -c -o $(BUILDDIR)/CallbackManager.o

$(OUTPUT_DIR):
	mkdir $(OUTPUT_DIR)
	cp $(BASE_DIR)/XPLMServer/pluginConfig.json $(OUTPUT_DIR)/pluginConfig.json
	mkdir $(OUTPUT_DIR)/64/

$(BUILDDIR):
	mkdir $(BUILDDIR)

publish:
	cp -r $(BUILDDIR)/XPLMServer $(XPLANE)/Resources/plugins/

clean:
	@echo Cleaning out everything.
	rm -rf $(BUILDDIR)
	rm -rf $(OUTPUT_DIR)/64

-include $(ALL_DEPS64)
