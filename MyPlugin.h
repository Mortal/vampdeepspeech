
// This is a skeleton file for use in creating your own plugin
// libraries.  Replace MyPlugin and myPlugin throughout with the name
// of your first plugin class, and fill in the gaps as appropriate.


// Remember to use a different guard symbol in each header!
#ifndef _MY_PLUGIN_H_
#define _MY_PLUGIN_H_

#include <vamp-sdk/Plugin.h>

using std::string;


class MyPlugin : public Vamp::Plugin
{
public:
    MyPlugin(float inputSampleRate);
    virtual ~MyPlugin();

    string getIdentifier() const;
    string getName() const;
    string getDescription() const;
    string getMaker() const;
    int getPluginVersion() const;
    string getCopyright() const;

    InputDomain getInputDomain() const;

    OutputList getOutputDescriptors() const;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    size_t m_stepSize;
    int m_pid;
    int m_toPython;
    int m_fromPython;
    int m_count;
};



#endif
