
// This is a skeleton file for use in creating your own plugin
// libraries.  Replace MyPlugin and myPlugin throughout with the name
// of your first plugin class, and fill in the gaps as appropriate.


#include "MyPlugin.h"
#include <unistd.h>
#include <sstream>
#include "vds_error.h"
#include "Backend.h"



MyPlugin::MyPlugin(float inputSampleRate) :
    Plugin(inputSampleRate)
    // Also be sure to set your plugin parameters (presumably stored
    // in member variables) to their default values here -- the host
    // will not do that for you
{
    m_pid = m_toPython = m_fromPython = -1;
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

    int fd1[2];
    int fd2[2];
    int r = pipe(fd1);
    if (r == -1) vds_error("No pipe()");
    r = pipe(fd2);
    if (r == -1) vds_error("No pipe()");
    int pid = fork();
    if (pid == -1) vds_error("No fork()");
    if (pid == 0) {
	close(fd1[1]);
	close(fd2[0]);
	r = dup2(fd1[0], 0);
	if (r == -1) vds_error("No dup2()");
	r = dup2(fd2[1], 1);
	if (r == -1) vds_error("No dup2()");
	char path[] = "/home/rav/work/vampdeepspeech/deepspeechdirect.py";
	execl(path, path, 0);
	vds_error("No execve()");
    }
    close(fd1[0]);
    close(fd2[1]);
    m_pid = pid;
    m_toPython = fd1[1];
    m_fromPython = fd2[0];

    return true;
}

void
MyPlugin::reset()
{
    // Clear buffers, reset stored values, etc
    if (m_toPython != -1) close(m_toPython);
    m_toPython = -1;
    if (m_fromPython != -1) close(m_fromPython);
    m_fromPython = -1;
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
    int s = 0;
    int r;
    while (s < m_stepSize) {
	r = ::write(m_toPython, inputBuffers[0], (m_stepSize - s) * sizeof(float));
	if (r < 0) vds_error("write failed");
	s += r;
    }
    return FeatureSet();
}

MyPlugin::FeatureSet
MyPlugin::getRemainingFeatures()
{
    close(m_toPython);
    m_toPython = -1;
    std::string buf;
    buf.resize(1024);
    std::stringstream res;
    int r = 0;
    do {
	r = read(m_fromPython, &buf[0], buf.size());
	if (r > 0) res.write(&buf[0], r);
    } while (r > 0);
    if (r < 0) vds_error("read failed");
    close(m_fromPython);
    m_fromPython = -1;

    Feature feature;
    feature.hasTimestamp = true;
    feature.timestamp = m_start;
    feature.hasDuration = true;
    feature.duration = Vamp::RealTime(m_duration, 0) / m_inputSampleRate;
    feature.label = res.str();
    FeatureSet result;
    result[0].push_back(feature);
    return result;
}

