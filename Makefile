PLUGIN_LIBRARY_NAME := vampdeepspeech

PLUGIN_SOURCES := MyPlugin.cpp plugins.cpp Backend.cpp

PLUGIN_HEADERS := MyPlugin.h Backend.h vds_error.h

CXXFLAGS := -g -I/usr/include/vamp-sdk -Wall -fPIC
PLUGIN_EXT := .so
LDFLAGS := -shared -Wl,-soname=$(PLUGIN_LIBRARY_NAME)$(PLUGIN_EXT) -lvamp-sdk -Wl,--version-script=vamp-plugin.map

##  All of the above

PLUGIN_OBJECTS := $(PLUGIN_SOURCES:.cpp=.o)
PLUGIN_OBJECTS := $(PLUGIN_OBJECTS:.c=.o)

$(PLUGIN_LIBRARY_NAME)$(PLUGIN_EXT): $(PLUGIN_OBJECTS)
	   $(CXX) -o $@ $^ $(LDFLAGS)

$(PLUGIN_OBJECTS): $(PLUGIN_HEADERS)

clean:
	$(RM) *.o

run: vampdeepspeech.so
	cp -a $^ ~/vamp/
	$(RM) ~/.audacity-data/pluginregistry.cfg
	audacity
