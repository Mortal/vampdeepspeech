PLUGIN_LIBRARY_NAME := vampdeepspeech

PLUGIN_SOURCES := DeepSpeechPlugin.cpp plugins.cpp Backend.cpp SilenceDetection.cpp

PLUGIN_HEADERS := DeepSpeechPlugin.h Backend.h vds_error.h SilenceDetection.h

DS_RUN_PATH ?= "`pwd`/deepspeech-native-client"
DS_INCLUDE_PATH ?= "`pwd`/DeepSpeech/native_client"
CPPFLAGS := -DVDS_ROOT=\""`pwd`"\" -I${DS_INCLUDE_PATH} -I/usr/include/vamp-sdk
CXXFLAGS := -std=gnu++14 -g -Wall -fPIC
PLUGIN_EXT := .so
LDFLAGS := -shared -Wl,-soname=$(PLUGIN_LIBRARY_NAME)$(PLUGIN_EXT) -lvamp-sdk -Wl,--version-script=vamp-plugin.map
LDFLAGS += -L${DS_RUN_PATH} -Wl,-rpath=${DS_RUN_PATH} -ldeepspeech_utils -ltensorflow_framework -ldeepspeech -ltensorflow_cc


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
