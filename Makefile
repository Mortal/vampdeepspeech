PLUGIN_LIBRARY_NAME := vampdeepspeech

PLUGIN_SOURCES := DeepSpeechPlugin.cpp plugins.cpp Backend.cpp SilenceDetection.cpp

PLUGIN_HEADERS := DeepSpeechPlugin.h Backend.h vds_error.h SilenceDetection.h

CPPFLAGS := -DVDS_ROOT=\""`pwd`"\"
CXXFLAGS := -std=gnu++14 -g -I/usr/include/vamp-sdk -Wall -fPIC
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
.PHONY: clean

install: vampdeepspeech.so
	mkdir -p ~/vamp
	cp -at ~/vamp $^
	$(RM) ~/.audacity-data/pluginregistry.cfg
.PHONY: install

run: install
	audacity
.PHONY: run
