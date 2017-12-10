
// This is a skeleton file for use in creating your own plugin
// libraries.  Replace MyPlugin and myPlugin throughout with the name
// of your first plugin class, and fill in the gaps as appropriate.


#include "MyPlugin.h"
#include <sstream>
#include "vds_error.h"



MyPlugin::MyPlugin(float inputSampleRate) :
    Plugin(inputSampleRate)
    // Also be sure to set your plugin parameters (presumably stored
    // in member variables) to their default values here -- the host
    // will not do that for you
{
    if (inputSampleRate != 16000) vds_error("Sample rate must be 16KHz");
    m_count = 0;
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
    m_count = 0;
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    m_stepSize = std::min(stepSize, blockSize);

    m_backend.reset(
	new vds::Backend(
	    "/home/rav/work/vampdeepspeech/deepspeechdirect.py"));

    return true;
}

void
MyPlugin::reset()
{
    // Clear buffers, reset stored values, etc
    m_backend.reset();
    m_count = 0;
}

MyPlugin::FeatureSet
MyPlugin::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    // Do actual work!
    if (m_count++ == 0) {
	m_start = timestamp;
	m_duration = 0;
    }
    m_duration += m_stepSize;
    m_backend->feed(inputBuffers[0], m_stepSize);
    return FeatureSet();
}

MyPlugin::FeatureSet
MyPlugin::getRemainingFeatures()
{
    std::string res = m_backend->infer();
    m_backend.reset();

    Feature feature;
    feature.hasTimestamp = true;
    feature.timestamp = m_start;
    feature.hasDuration = true;
    feature.duration = Vamp::RealTime(m_duration, 0) / m_inputSampleRate;
    feature.label = res;
    FeatureSet result;
    result[0].push_back(feature);
    return result;
}

