#include "DeepSpeechPlugin.h"
#include <sstream>
#include "vds_error.h"

DeepSpeechPlugin::DeepSpeechPlugin(float inputSampleRate) :
    Plugin(inputSampleRate)
    // Also be sure to set your plugin parameters (presumably stored
    // in member variables) to their default values here -- the host
    // will not do that for you
{
    if (inputSampleRate != 16000) vds_error("Sample rate must be 16KHz");
    m_count = 0;
}

DeepSpeechPlugin::~DeepSpeechPlugin()
{
}

std::string
DeepSpeechPlugin::getIdentifier() const
{
    return "vampdeepspeech";
}

std::string
DeepSpeechPlugin::getName() const
{
    return "DeepSpeech";
}

std::string
DeepSpeechPlugin::getDescription() const
{
    return "Run DeepSpeech on selected audio";
}

std::string
DeepSpeechPlugin::getMaker() const
{
    return "Mathias Rav";
}

int
DeepSpeechPlugin::getPluginVersion() const
{
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

std::string
DeepSpeechPlugin::getCopyright() const
{
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "GPL";
}

DeepSpeechPlugin::InputDomain
DeepSpeechPlugin::getInputDomain() const
{
    return TimeDomain;
}

DeepSpeechPlugin::OutputList
DeepSpeechPlugin::getOutputDescriptors() const
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
DeepSpeechPlugin::initialise(size_t channels, size_t stepSize, size_t blockSize)
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
DeepSpeechPlugin::reset()
{
    // Clear buffers, reset stored values, etc
    m_backend->clear();
    m_count = 0;
}

DeepSpeechPlugin::FeatureSet
DeepSpeechPlugin::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
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

DeepSpeechPlugin::FeatureSet
DeepSpeechPlugin::getRemainingFeatures()
{
    std::string res = m_backend->infer();

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
