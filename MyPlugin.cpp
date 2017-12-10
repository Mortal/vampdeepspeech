
// This is a skeleton file for use in creating your own plugin
// libraries.  Replace MyPlugin and myPlugin throughout with the name
// of your first plugin class, and fill in the gaps as appropriate.


#include "MyPlugin.h"


MyPlugin::MyPlugin(float inputSampleRate) :
    Plugin(inputSampleRate)
    // Also be sure to set your plugin parameters (presumably stored
    // in member variables) to their default values here -- the host
    // will not do that for you
{
}

MyPlugin::~MyPlugin()
{
}

string
MyPlugin::getIdentifier() const
{
    return "vampdeepspeech";
}

string
MyPlugin::getName() const
{
    return "DeepSpeech";
}

string
MyPlugin::getDescription() const
{
    // Return something helpful here!
    return "Run DeepSpeech on selected audio";
}

string
MyPlugin::getMaker() const
{
    // Your name here
    return "Mathias Rav";
}

int
MyPlugin::getPluginVersion() const
{
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string
MyPlugin::getCopyright() const
{
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "GPL";
}

MyPlugin::InputDomain
MyPlugin::getInputDomain() const
{
    return TimeDomain;
}

MyPlugin::OutputList
MyPlugin::getOutputDescriptors() const
{
    OutputList list;

    // Refer to Audacity's VampEffectsModule::FindPlugins
    // for important limitations on this
    OutputDescriptor d;
    d.identifier = "transcription";
    d.name = "Transcriptions";
    d.description = "The transcribed text";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 0;
    d.hasKnownExtents = false;
    d.isQuantized = true;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.hasDuration = false;
    list.push_back(d);

    return list;
}

bool
MyPlugin::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    // Real initialisation work goes here!

    return true;
}

void
MyPlugin::reset()
{
    // Clear buffers, reset stored values, etc
}

MyPlugin::FeatureSet
MyPlugin::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    // Do actual work!
    return FeatureSet();
}

MyPlugin::FeatureSet
MyPlugin::getRemainingFeatures()
{
    return FeatureSet();
}

