#include "PluginProcessor.h"
#include "PluginEditor.h"

#define OVERLAP_0 4
#define OVERLAP_1 8
#define OVERLAP_2 16
#define OVERLAP_3 32

#define FFT_SIZE_COEFF 23

NLDenoiserAudioProcessor::NLDenoiserAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                       ),
      _parameters(*this, nullptr, "PARAMETERS",
                 {
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"ratio", 700}, "Ratio", 0.0f, 100.0f, 100.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"threshold", 700}, "Threshold", 0.0f, 100.0f, 0.1f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"transientBoost", 700}, "Transient Boost", 0.0f, 100.0f, 0.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"residualNoise", 700}, "Residual Noise", 0.0f, 100.0f, 0.0f),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"learnModeParamID", 700}, "Learn Mode", false),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"noiseOnlyParamID", 700}, "Noise Only", false),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"softDenoiseParamID", 700}, "Soft Denoise", false),
                     std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{"quality", 700}, "Quality",
            juce::StringArray{"1 - Fast", "2", "3", "4 - Best"}, 0)
                 })
#endif
{
}

NLDenoiserAudioProcessor::~NLDenoiserAudioProcessor()
{
    for (int i = 0; i < _overlapAdds.size(); i++)
        delete _overlapAdds[i];

    for (int i = 0; i < _processors.size(); i++)
        delete _processors[i];
}

const juce::String NLDenoiserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NLDenoiserAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NLDenoiserAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NLDenoiserAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NLDenoiserAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NLDenoiserAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NLDenoiserAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NLDenoiserAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String NLDenoiserAudioProcessor::getProgramName(int index)
{
    return {};
}

void NLDenoiserAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void NLDenoiserAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    int numInputChannels = getTotalNumInputChannels();
    
    int fftSize = juce::nextPowerOfTwo(sampleRate/FFT_SIZE_COEFF);

    auto quality = _parameters.getRawParameterValue("quality")->load();
    int overlap = getOverlap(quality);

    auto threshold = _parameters.getRawParameterValue("threshold")->load();
    
    if (_overlapAdds.size() != numInputChannels)
    {
        for (int i = 0; i < _overlapAdds.size(); i++)
            delete _overlapAdds[i];
        _overlapAdds.clear();
        
        for (int i = 0; i < _processors.size(); i++)
            delete _processors[i];
        _processors.clear();
        
        for (int i = 0; i < numInputChannels; i++)
        {
            DenoiserProcessor *processor = new DenoiserProcessor(fftSize/2 + 1, overlap, threshold);
            _processors.push_back(processor);
            
            OverlapAdd *overlapAdd = new OverlapAdd(fftSize, overlap, true, true);
            overlapAdd->addProcessor(processor);
            _overlapAdds.push_back(overlapAdd);
        }
    }

    for (int i = 0; i < _overlapAdds.size(); i++)
    {
        _overlapAdds[i]->setFftSize(fftSize);
        _overlapAdds[i]->setOverlap(overlap);
    }

    for (int i = 0; i < _processors.size(); i++)
        _processors[i]->reset(fftSize/2 + 1, overlap, sampleRate);

    // Update latency
    if (!_processors.empty())
    {
        int latency = _processors[0]->getLatency();
        setLatencySamples(latency);
        updateHostDisplay();
    }
}

void NLDenoiserAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NLDenoiserAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif
    
void NLDenoiserAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Retrieve parameter values
    auto ratio = _parameters.getRawParameterValue("ratio")->load();
    auto threshold = _parameters.getRawParameterValue("threshold")->load();
    auto transientBoost = _parameters.getRawParameterValue("transientBoost")->load();
    auto residualNoise = _parameters.getRawParameterValue("residualNoise")->load();
    auto learnMode = _parameters.getRawParameterValue("learnModeParamID")->load();
    auto noiseOnly = _parameters.getRawParameterValue("noiseOnlyParamID")->load();
    auto softDenoise = _parameters.getRawParameterValue("softDenoiseParamID")->load();
    auto quality = _parameters.getRawParameterValue("quality")->load();
    
    ratio *= 0.01;
    
    bool qualityChanged = (quality != _prevQualityParam);
    _prevQualityParam = quality;
                    
    // Set parameters
    for (int i = 0; i < _processors.size(); i++)
    {
        _processors[i]->setThreshold(threshold);
        _processors[i]->setResNoiseThrs(residualNoise);
        _processors[i]->setBuildingNoiseStatistics(learnMode);
        _processors[i]->setAutoResNoise(softDenoise);

        if (qualityChanged)
        {            
            int overlap = getOverlap(quality);

            _processors[i]->setOverlap(overlap);
            _overlapAdds[i]->setOverlap(overlap);

            // Update latency
            if (!_processors.empty())
            {
                int latency = _processors[0]->getLatency();
                setLatencySamples(latency);
                updateHostDisplay();
            }
        }
    }
    
    // Process
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        vector<float> vecBuf;
        vecBuf.resize(buffer.getNumSamples());
        memcpy(vecBuf.data(), channelData, buffer.getNumSamples()*sizeof(float));
        
        _overlapAdds[channel]->feed(vecBuf);

        vector<float> outBuf;
        int numSamplesToFlush = _overlapAdds[channel]->getOutSamples(&outBuf, buffer.getNumSamples());
        _overlapAdds[channel]->flushOutSamples(numSamplesToFlush);

        vector<float> noiseBuf;
        getNoiseBuf(&noiseBuf, channelData, outBuf);

        applyRatio(ratio, &outBuf, channelData, noiseBuf);

        if (noiseOnly > 0.5)
            outBuf = noiseBuf;
        
        memcpy(channelData, outBuf.data(), buffer.getNumSamples()*sizeof(float));
    }
}

bool NLDenoiserAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NLDenoiserAudioProcessor::createEditor()
{
    return new NLDenoiserAudioProcessorEditor (*this);
}

void NLDenoiserAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    _parameters.state.writeToStream(stream);
}

void NLDenoiserAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    _parameters.state = juce::ValueTree::readFromStream(stream);
}

int
NLDenoiserAudioProcessor::getOverlap(int quality)
{
    switch(quality)
    {
        case 0:
            return OVERLAP_0;
            break;
            
        case 1:
            return OVERLAP_1;
            break;
            
        case 2:
            return OVERLAP_2;
            break;

        case 3:
            return OVERLAP_3;
            break;
            
        default:
            return OVERLAP_0;
    }
}

void
NLDenoiserAudioProcessor::getNoiseBuf(vector<float> *noiseBuf, float *inputBuf, const vector<float> &outputBuf)
{
    noiseBuf->resize(outputBuf.size());

    for (int i = 0; i < noiseBuf->size(); i++)
        (*noiseBuf)[i] = inputBuf[i] - outputBuf[i];
}

void
NLDenoiserAudioProcessor::applyRatio(float ratio, vector<float> *outputBuf,
                                     float *channelData, const vector<float> &noiseBuf)
{
    for (int i = 0; i < outputBuf->size(); i++)
        (*outputBuf)[i] = channelData[i] + ratio*noiseBuf[i];
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NLDenoiserAudioProcessor();
}
