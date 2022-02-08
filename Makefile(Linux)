SDK			:=	/home/rdessart/Lib/XPSDK/213
TARGET		:= XPLMServer
BASE_DIR    := /home/rdessart/Projects/C/XPLMServer
BUILDDIR	:=	$(BASE_DIR)/build
CALLBACKMANAGER := $(BASE_DIR)/CallbackManager
DEFAULTCALLBACK := $(BASE_DIR)/DefaultCallbacks
NETWORKING      := $(BASE_DIR)/Networking
XPLMServer 	    := $(BASE_DIR)/XPLMServer

INCLUDES = \
	-I$(SDK)/CHeaders/XPLM\
	-I$(CALLBACKMANAGER)/include\
	-I$(NETWORKING)/include\

DEFINES = -DXPLM200=1 -DXPLM210=1 -DAPL=0 -DIBM=0 -DLIN=1

############################################################################


VPATH = $(SDK)

CSOURCES	:= $(filter %.c, $(SOURCES))
CXXSOURCES	:= $(filter %.cpp, $(SOURCES))

CDEPS64			:= $(patsubst %.c, $(BUILDDIR)/obj64/%.cdep, $(CSOURCES))
CXXDEPS64		:= $(patsubst %.cpp, $(BUILDDIR)/obj64/%.cppdep, $(CXXSOURCES))
COBJECTS64		:= $(patsubst %.c, $(BUILDDIR)/obj64/%.o, $(CSOURCES))
CXXOBJECTS64	:= $(patsubst %.cpp, $(BUILDDIR)/obj64/%.o, $(CXXSOURCES))
ALL_DEPS64		:= $(sort $(CDEPS64) $(CXXDEPS64))
ALL_OBJECTS64	:= $(sort $(COBJECTS64) $(CXXOBJECTS64))

CFLAGS := $(DEFINES) $(INCLUDES) -std=c++20 -fPIC -fvisibility=hidden

###XPLMServer

$(BUILDDIR)/lin.xpl : $(BUILDDIR)/UDPServer.o $(BUILDDIR)/Dataref.o $(BUILDDIR)/Logger.o $(BUILDDIR)/utils.o $(BUILDDIR)/CallbackManager.o $(BUILDDIR)/XPLMServer.o
	echo $(BUILDDIR)/Networking.a
	gcc -m64 -static-libgcc -shared -Wl,--version-script=exports.txt $(BUILDDIR)/UDPServer.o $(BUILDDIR)/Dataref.o $(BUILDDIR)/Logger.o $(BUILDDIR)/utils.o $(BUILDDIR)/CallbackManager.o $(BUILDDIR)/XPLMServer.o  -o $(BUILDDIR)/lin.xpl

$(BUILDDIR)/XPLMServer.o : $(XPLMServer)/src/main.cpp
	g++ $(CFLAGS) $(XPLMServer)/src/main.cpp -c -o $(BUILDDIR)/XPLMServer.o

### Networking

$(BUILDDIR)/libNetworking.a: $(BUILDDIR)/UDPServer.o 
	ar rcs $(BUILDDIR)/libNetworking.a $(BUILDDIR)/UDPServer.o

$(BUILDDIR)/UDPServer.o : $(NETWORKING)/src/UDPServer.cpp
	mkdir $(BUILDDIR)
	g++ $(CFLAGS) $(NETWORKING)/src/UDPServer.cpp -c -o $(BUILDDIR)/UDPServer.o

###DefaultCallback

$(BUILDDIR)/DefaultCallbacks.so : $(BUILDDIR)/DefaultCallbacks.o
	g++ -shared -o $(BUILDDIR)/DefaultCallbacks.so $(BUILDDIR)/DefaultCallbacks.o

### CallbackManager

$(BUILDDIR)/libCallbackManager.a: $(BUILDDIR)/Dataref.o $(BUILDDIR)/Logger.o $(BUILDDIR)/utils.o $(BUILDDIR)/CallbackManager.o
	ar rcs $(BUILDDIR)/libCallbackManager.a $(BUILDDIR)/Dataref.o $(BUILDDIR)/Logger.o $(BUILDDIR)/utils.o $(BUILDDIR)/CallbackManager.o

$(BUILDDIR)/Dataref.o : $(CALLBACKMANAGER)/src/Dataref.cpp
	g++ $(CFLAGS) $(CALLBACKMANAGER)/src/Dataref.cpp -c -o $(BUILDDIR)/Dataref.o

$(BUILDDIR)/Logger.o : $(CALLBACKMANAGER)/src/Logger.cpp
	g++ $(CFLAGS) $(CALLBACKMANAGER)/src/Logger.cpp -c -o $(BUILDDIR)/Logger.o

$(BUILDDIR)/utils.o : $(CALLBACKMANAGER)/src/utils.cpp
	g++ $(CFLAGS) $(CALLBACKMANAGER)/src/utils.cpp -c -o $(BUILDDIR)/utils.o

$(BUILDDIR)/CallbackManager.o : $(CALLBACKMANAGER)/src/CallbackManager.cpp
	g++ $(CFLAGS) $(CALLBACKMANAGER)/src/CallbackManager.cpp -c -o $(BUILDDIR)/CallbackManager.o

$(BUILDDIR)/DefaultCallbacks.o : $(DEFAULTCALLBACK)/src/Callbacks.cpp
	echo $(INCLUDES)
	g++ $(CFLAGS) $(DEFAULTCALLBACK)/src/Callbacks.cpp -c -o $(BUILDDIR)/DefaultCallbacks.o

clean:
	@echo Cleaning out everything.
	rm -rf $(BUILDDIR)

-include $(ALL_DEPS64)


