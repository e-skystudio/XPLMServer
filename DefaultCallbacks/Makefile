BUILDDIR	:=	./build
SDK			:=	/home/rdessart/Lib/XPSDK/
TARGET		:= XPLMServer
BASE_DIR    := /home/rdessart/Projects/C/XPLMServer
CALLBACKMANAGER := $(BASE_DIR)/CallbackManager
DEFAULTCALLBACK := $(BASE_DIR)/DefaultCallbacks

LIBS = 

INCLUDES = \
	-I$(SDK)includes/XPLM\
	-I$(CALLBACKMANAGER)/include


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

### CallbackManager
$(BUILDDIR)/DefaultCallbacks.so : $(BUILDDIR)/DefaultCallbacks.o
	g++ -shared -o $(BUILDDIR)/DefaultCallbacks.so $(BUILDDIR)/DefaultCallbacks.o

$(BUILDDIR)/DefaultCallbacks.o : $(DEFAULTCALLBACK)/src/Callbacks.cpp
	mkdir $(BUILDDIR)
	g++ $(CFLAGS) $(DEFAULTCALLBACK)/src/Callbacks.cpp -c -o $(BUILDDIR)/DefaultCallbacks.o

clean:
	@echo Cleaning out everything.
	rm -rf $(BUILDDIR)

-include $(ALL_DEPS64)


