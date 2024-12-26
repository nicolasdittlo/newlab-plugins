#pragma once

#include <JuceHeader.h>

#include <OverlapAdd.h>
#include <DenoiserProcessor.h>

class NLDenoiserAudioProcessor  : public juce::AudioProcessor
{
public:
    using SampleRateChangeListener = std::function<void(double, int)>;
    
    NLDenoiserAudioProcessor();
    ~NLDenoiserAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
        
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void setSampleRateChangeListener(SampleRateChangeListener listener);

    void getBuffers(vector<float> *signalBuffer,
                    vector<float> *noiseBuffer,
                    vector<float> *noiseProfileBuffer);
    
public:
    juce::AudioProcessorValueTreeState _parameters;
    
private:
    int getOverlap(int quality);

    void getNoiseBuf(vector<float> *noiseBuf, float *inputBuf, const vector<float> &outputBuf);

    void applyRatio(float ratio, vector<float> *outputBuf,
                    float *channelData, const vector<float> &noiseBuf);
        
    vector<OverlapAdd *> _overlapAdds;
    vector<DenoiserProcessor *> _processors;

    int _prevQualityParam = 0;

    double _sampleRate = 0.0;
    SampleRateChangeListener _sampleRateChangeListener = nullptr;

    vector<float> _signalBuffer;
    vector<float> _noiseBuffer;
    vector<float> _noiseProfileBuffer;
    
    std::mutex _curvesMutex;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NLDenoiserAudioProcessor)
};
