#ifndef DEEP_SPEECH_PLUGIN_H
#define DEEP_SPEECH_PLUGIN_H

#include <vamp-sdk/Plugin.h>
#include "Backend.h"

class DeepSpeechPlugin : public Vamp::Plugin
{
public:
    DeepSpeechPlugin(float inputSampleRate);
    virtual ~DeepSpeechPlugin();

    std::string getIdentifier() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getMaker() const;
    int getPluginVersion() const;
    std::string getCopyright() const;

    InputDomain getInputDomain() const;

    OutputList getOutputDescriptors() const;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    size_t m_stepSize;
    std::unique_ptr<vds::Backend> m_backend;
    int m_count;
    Vamp::RealTime m_start;
    size_t m_duration;
};

#endif // DEEP_SPEECH_PLUGIN_H
