// vim:set sw=4 noet:
#include "DeepSpeechPlugin.h"
#include <sstream>
#include "vds_error.h"

DeepSpeechPlugin::DeepSpeechPlugin(float inputSampleRate) :
    Plugin(inputSampleRate)
    // Also be sure to set your plugin parameters (presumably stored
    // in member variables) to their default values here -- the host
    // will not do that for you
{
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
    if (m_inputSampleRate != 16000) {
	std::cerr << "DeepSpeech error: Sample rate must be 16KHz" << std::endl;
	return false;
    }
    m_count = 0;
    if (channels < getMinChannelCount() || channels > getMaxChannelCount()) {
	std::cerr << "DeepSpeech error: Unsupported number of channels" << std::endl;
	return false;
    }

    m_stepSize = std::min(stepSize, blockSize);

    try {
	m_backend = vds::Backend::make(VDS_ROOT);
    } catch (vds::configuration_error e) {
	std::cerr << "DeepSpeech error: " << e.what() << std::endl;
	return false;
    }
    m_silence.reset(
	new vds::SilenceDetection());

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
    if (m_silence->silent(inputBuffers[0], m_stepSize)) {
	return infer();
    } else {
	m_duration += m_stepSize;
	m_backend->feed(inputBuffers[0], m_stepSize);
	return FeatureSet();
    }
}

DeepSpeechPlugin::FeatureSet
DeepSpeechPlugin::getRemainingFeatures() {
    return infer();
}

DeepSpeechPlugin::FeatureSet
DeepSpeechPlugin::infer()
{
    std::string res = m_backend->infer();
    if (res.size() == 0) {
	m_count = 0;
	return FeatureSet();
    }

    Feature feature;
    feature.hasTimestamp = true;
    feature.timestamp = m_start;
    feature.hasDuration = true;
    feature.duration = Vamp::RealTime(m_duration, 0) / m_inputSampleRate;
    feature.label = res;
    FeatureSet result;
    result[0].push_back(feature);
    m_count = 0;
    return result;
}
