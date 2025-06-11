/* Copyright (C) 2025 Nicolas Dittlo <bluelab.plugins@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this software; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "OverlapAdd.h"
#include "STNProcessor.h"
#include "BufProcessor.h"
#include "Utils.h"
#include "ParamSmoother.h"
#include "CrossoverSplitterNBands.h"
#include "Delay.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define OVERLAP 4

#define FFT_SIZE_COEFF 23

#define DEFAULT_SPLIT_FREQ 20.0
#define DEFAULT_SPLIT_FREQ_SMOOTH_TIME_MS 280.0

#define MIN_SPLIT_FREQ 20.0

BLSTNAudioProcessor::BLSTNAudioProcessor()
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
            juce::ParameterID{"sinesMix", 701}, "Sines", -12.0f, 12.0f, 0.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"transientsMix", 701}, "Transients", -12.0f, 12.0f, 0.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"noiseMix", 701}, "Noise", -12.0f, 12.0f, 0.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"outGain", 701}, "Out Gain", -12.0f, 12.0f, 0.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"wetFreq", 701}, "Wet Freq", 20.0f, 20000.0f, 20.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"wetGain", 701}, "Wet Gain", -12.0f, 12.0f, 0.0f),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"soloSines", 701}, "Solo Sines", false),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"muteSines", 701}, "Mute Sines", false),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"soloTransients", 701}, "Solo Transients", false),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"muteTransients", 701}, "Mute Transients", false),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"soloNoise", 701}, "Solo Noise", false),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"muteNoise", 701}, "Mute Noise", false)
                 })
#endif
{
    float sampleRate = 44100.0;
    _sampleRate = -1.0;
    
    float defaultSplitFreq = DEFAULT_SPLIT_FREQ;
    float splitFreqSmoothTime = DEFAULT_SPLIT_FREQ_SMOOTH_TIME_MS;

    // Adjust, because smoothing is done only once in each processBlock()
    int blockSize = 512;
    splitFreqSmoothTime /= blockSize;
    
    _splitFreqSmoother = new ParamSmoother(sampleRate, defaultSplitFreq,
                                           splitFreqSmoothTime);
}

BLSTNAudioProcessor::~BLSTNAudioProcessor()
{
    for (int i = 0; i < _displaySinesOverlapAdds.size(); i++)
        delete _displaySinesOverlapAdds[i];

    for (int i = 0; i < _displaySinesProcessors.size(); i++)
        delete _displaySinesProcessors[i];

    for (int i = 0; i < _displayNoiseOverlapAdds.size(); i++)
        delete _displayNoiseOverlapAdds[i];

    for (int i = 0; i < _displaySinesProcessors.size(); i++)
        delete _displaySinesProcessors[i];
    
    for (int i = 0; i < _displayOutOverlapAdds.size(); i++)
        delete _displayOutOverlapAdds[i];

    for (int i = 0; i < _displayOutProcessors.size(); i++)
        delete _displayOutProcessors[i];

    for (int i = 0; i < _outGainSmoothers.size(); i++)
        delete _outGainSmoothers[i];

    for (int i = 0; i < _wetGainSmoothers.size(); i++)
        delete _wetGainSmoothers[i];

    delete _splitFreqSmoother;
        
    for (int i = 0; i < _bandSplittersIn.size(); i++)
        delete _bandSplittersIn[i];

    for (int i = 0; i < _bandSplittersOut.size(); i++)
        delete _bandSplittersOut[i];
        
    for (int i = 0; i < _inputDelays.size(); i++)
        delete _inputDelays[i];
}

const juce::String
BLSTNAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool
BLSTNAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool
BLSTNAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool
BLSTNAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double
BLSTNAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int
BLSTNAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int
BLSTNAudioProcessor::getCurrentProgram()
{
    return 0;
}

void
BLSTNAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String
BLSTNAudioProcessor::getProgramName(int index)
{
    return {};
}

void
BLSTNAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void
BLSTNAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{    
    int numInputChannels = getTotalNumInputChannels();
    
    int fftSize = Utils::nearestPowerOfTwo(sampleRate/FFT_SIZE_COEFF);
    
    if (sampleRate != _sampleRate)
    {
        _sampleRate = sampleRate;
        
        // Notify listener
        if (_sampleRateChangeListener != nullptr)
            _sampleRateChangeListener(sampleRate, fftSize/2 + 1);
    }

    for (int i = 0; i < _processors.size(); i++)
        _processors[i]->prepareToPlay(sampleRate);
    
    // Number of channels changed?
    if (_bandSplittersIn.size() != numInputChannels)
    {
        for (int i = 0; i < _bandSplittersIn.size(); i++)
            delete _bandSplittersIn[i];
        _bandSplittersIn.clear();
        
        for (int i = 0; i < _bandSplittersOut.size(); i++)
            delete _bandSplittersOut[i];
        _bandSplittersOut.clear();
            
        for (int i = 0; i < _inputDelays.size(); i++)
            delete _inputDelays[i];
        _inputDelays.clear();

        for (int i = 0; i < _outGainSmoothers.size(); i++)
            delete _outGainSmoothers[i];
        _outGainSmoothers.clear();

        for (int i = 0; i < _wetGainSmoothers.size(); i++)
            delete _wetGainSmoothers[i];
        _wetGainSmoothers.clear();

        // Sines
        for (int i = 0; i < _displaySinesOverlapAdds.size(); i++)
            delete _displaySinesOverlapAdds[i];
        _displaySinesOverlapAdds.clear();
        
        for (int i = 0; i < _displaySinesProcessors.size(); i++)
            delete _displaySinesProcessors[i];
        _displaySinesProcessors.clear();

        // Noise
        for (int i = 0; i < _displayNoiseOverlapAdds.size(); i++)
            delete _displayNoiseOverlapAdds[i];
        _displayNoiseOverlapAdds.clear();
        
        for (int i = 0; i < _displayNoiseProcessors.size(); i++)
            delete _displayNoiseProcessors[i];
        _displayNoiseProcessors.clear();
        
        // Out
        for (int i = 0; i < _displayOutOverlapAdds.size(); i++)
            delete _displayOutOverlapAdds[i];
        _displayOutOverlapAdds.clear();
        
        for (int i = 0; i < _displayOutProcessors.size(); i++)
            delete _displayOutProcessors[i];
        _displayOutProcessors.clear();
        
        for (int i = 0; i < numInputChannels; i++)
        {
            STNProcessor *stnProcessor = new STNProcessor();
            stnProcessor->prepareToPlay(sampleRate);
            _processors.push_back(stnProcessor);

            // Sines
            BufProcessor *displaySinesProcessor = new BufProcessor();
            _displaySinesProcessors.push_back(displaySinesProcessor);
            
            OverlapAdd *displaySinesOverlapAdd = new OverlapAdd(fftSize, OVERLAP, true, false);
            displaySinesOverlapAdd->addProcessor(displaySinesProcessor);
            _displaySinesOverlapAdds.push_back(displaySinesOverlapAdd);

            // Noise
            BufProcessor *displayNoiseProcessor = new BufProcessor();
            _displayNoiseProcessors.push_back(displayNoiseProcessor);
            
            OverlapAdd *displayNoiseOverlapAdd = new OverlapAdd(fftSize, OVERLAP, true, false);
            displayNoiseOverlapAdd->addProcessor(displayNoiseProcessor);
            _displayNoiseOverlapAdds.push_back(displayNoiseOverlapAdd);
            
            // Out
            BufProcessor *displayOutProcessor = new BufProcessor();
            _displayOutProcessors.push_back(displayOutProcessor);
            
            OverlapAdd *displayOutOverlapAdd = new OverlapAdd(fftSize, OVERLAP, true, false);
            displayOutOverlapAdd->addProcessor(displayOutProcessor);
            _displayOutOverlapAdds.push_back(displayOutOverlapAdd);
        }

        float splitFreqs[1] = { DEFAULT_SPLIT_FREQ };
        for (int i = 0; i < numInputChannels; i++)
        {
            CrossoverSplitterNBands *splitter = new CrossoverSplitterNBands(2, splitFreqs, sampleRate);
            _bandSplittersIn.push_back(splitter);
        }
        
        for (int i = 0; i < numInputChannels; i++)
        {
            CrossoverSplitterNBands *splitter = new CrossoverSplitterNBands(2, splitFreqs, sampleRate);
            _bandSplittersOut.push_back(splitter);
                
        }
    
        auto wetFreq = _parameters.getRawParameterValue("wetFreq")->load();
        setSplitFreq(wetFreq);
        
        for (int i = 0; i < numInputChannels; i++)
        {
            Delay *delay = new Delay(fftSize);
            _inputDelays.push_back(delay);
        }

        for (int i = 0; i < numInputChannels; i++)
        {
            float defaultOutGain = 1.0;
            ParamSmoother *outGainSmoother = new ParamSmoother(sampleRate, defaultOutGain);
            _outGainSmoothers.push_back(outGainSmoother); 
        }

        for (int i = 0; i < numInputChannels; i++)
        {
            float defaultWetGain = 1.0;
            ParamSmoother *wetGainSmoother = new ParamSmoother(sampleRate, defaultWetGain);
            _wetGainSmoothers.push_back(wetGainSmoother); 
        }
    }

    // Sines
    for (int i = 0; i < _displaySinesOverlapAdds.size(); i++)
    {
        _displaySinesOverlapAdds[i]->setFftSize(fftSize);
        _displaySinesOverlapAdds[i]->setOverlap(OVERLAP);
    }
    
    // Noise
    for (int i = 0; i < _displayNoiseOverlapAdds.size(); i++)
    {
        _displayNoiseOverlapAdds[i]->setFftSize(fftSize);
        _displayNoiseOverlapAdds[i]->setOverlap(OVERLAP);
    }
    
    // Out
    for (int i = 0; i < _displayOutOverlapAdds.size(); i++)
    {
        _displayOutOverlapAdds[i]->setFftSize(fftSize);
        _displayOutOverlapAdds[i]->setOverlap(OVERLAP);
    }

    auto outGain = _parameters.getRawParameterValue("outGain")->load();
    outGain = Utils::DBToAmp(outGain);
    for (int i = 0; i < _outGainSmoothers.size(); i++)
    {
        _outGainSmoothers[i]->resetToTargetValue(outGain);
        _outGainSmoothers[i]->reset(sampleRate);
    }
    
    auto wetGain = _parameters.getRawParameterValue("wetGain")->load();
    wetGain = Utils::DBToAmp(wetGain);
    for (int i = 0; i < _wetGainSmoothers.size(); i++)
    {
        _wetGainSmoothers[i]->resetToTargetValue(wetGain);
        _wetGainSmoothers[i]->reset(sampleRate);
    }
    
    auto wetFreq = _parameters.getRawParameterValue("wetFreq")->load();
    setSplitFreq(wetFreq);
    _splitFreqSmoother->resetToTargetValue(wetFreq);
    _splitFreqSmoother->reset(sampleRate);
    
    // Update latency
    int latency = getLatency();
    setLatencySamples(latency);
    updateHostDisplay();

    // Update the delays
    for (int i = 0; i < _inputDelays.size(); i++)
        _inputDelays[i]->setDelay(latency);
    
    for (int i = 0; i < _bandSplittersIn.size(); i++)
        _bandSplittersIn[i]->reset(sampleRate);
    
    for (int i = 0; i < _bandSplittersOut.size(); i++)
        _bandSplittersOut[i]->reset(sampleRate);
}

void
BLSTNAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool
BLSTNAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void
BLSTNAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
        buffer.clear(i, 0, buffer.getNumSamples());

    // Retrieve parameter values
    auto sinesMix = _parameters.getRawParameterValue("sinesMix")->load();
    auto transientsMix = _parameters.getRawParameterValue("transientsMix")->load();
    auto noiseMix = _parameters.getRawParameterValue("noiseMix")->load();
    auto outGain = _parameters.getRawParameterValue("outGain")->load();
    auto wetFreq = _parameters.getRawParameterValue("wetFreq")->load();
    auto wetGain = _parameters.getRawParameterValue("wetGain")->load();
    auto soloSines = _parameters.getRawParameterValue("soloSines")->load();
    auto muteSines = _parameters.getRawParameterValue("muteSines")->load();
    auto soloTransients = _parameters.getRawParameterValue("soloTransients")->load();
    auto muteTransients = _parameters.getRawParameterValue("muteTransients")->load();
    auto soloNoise = _parameters.getRawParameterValue("soloNoise")->load();
    auto muteNoise = _parameters.getRawParameterValue("muteNoise")->load();
    
    sinesMix = Utils::DBToAmp(sinesMix);
    transientsMix = Utils::DBToAmp(transientsMix);
    noiseMix = Utils::DBToAmp(noiseMix);
    outGain = Utils::DBToAmp(outGain);
    wetGain = Utils::DBToAmp(wetGain);

    bool solos[3] = { (soloSines > 0.5), (soloTransients > 0.5), (soloNoise > 0.5) };
    bool mutes[3] = { (muteSines > 0.5), (muteTransients > 0.5), (muteNoise > 0.5) };
    bool resultMutes[3];
    
    computeMutes(solos, mutes, resultMutes);
        
    // Set parameters
    for (int i = 0; i < _processors.size(); i++)
    {
        _processors[i]->setSinesMix(sinesMix);
        _processors[i]->setTransientsMix(transientsMix);
        _processors[i]->setNoiseMix(noiseMix);

        _processors[i]->setMuteSines(resultMutes[0]);
        _processors[i]->setMuteTransients(resultMutes[1]);
        _processors[i]->setMuteNoise(resultMutes[2]);
    }

    _splitFreqSmoother->setTargetValue(wetFreq);
    if (!_splitFreqSmoother->isStable())
    {
        float splitFreq = _splitFreqSmoother->process();
        
        setSplitFreq(splitFreq);
    }

    for (int i = 0; i < _wetGainSmoothers.size(); i++)
        _wetGainSmoothers[i]->setTargetValue(wetGain);

    for (int i = 0; i < _outGainSmoothers.size(); i++)
        _outGainSmoothers[i]->setTargetValue(outGain);
    
    // Process
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        vector<float> inBuf;
        inBuf.resize(buffer.getNumSamples());
        memcpy(inBuf.data(), channelData, buffer.getNumSamples()*sizeof(float));

        vector<float> outBuf;
        _processors[channel]->process(inBuf, &outBuf);

        // Splitter
        if (wetFreq >= MIN_SPLIT_FREQ)
        {
            // Split in
            vector<float> inLo;
            vector<float> inHi;
            
            vector<float> resultBufIn[2];
            _bandSplittersIn[channel]->split(inBuf, (vector<float> *)&resultBufIn);

            inLo = resultBufIn[0];
            inHi = resultBufIn[1];
            
            // Split out
            vector<float> outLo;
            vector<float> outHi;
            
            vector<float> resultBufOut[2];
            _bandSplittersOut[channel]->split(outBuf, resultBufOut);

            outLo = resultBufOut[0];
            outHi = resultBufOut[1];
                
            // Delay input
            _inputDelays[channel]->processSamples(&inLo);
        
            // Apply wet gain
            Utils::applyGain(outHi, &outHi, _wetGainSmoothers[channel]);

            // Sum
            Utils::addBuffers(&outBuf, inLo, outHi);
        }
        
        // Generate the output magnitudes
        if (channel == 0)
        {
            // sines
            vector<float> xs;
            _processors[channel]->getSinesBuffer(&xs);
            _displaySinesOverlapAdds[channel]->feed(xs);

            // noise
            vector<float> xn;
            _processors[channel]->getNoiseBuffer(&xn);
            _displayNoiseOverlapAdds[channel]->feed(xn);
            
            _displayOutOverlapAdds[channel]->feed(outBuf);
        }

        // Apply out gain
        Utils::applyGain(outBuf, &outBuf, _outGainSmoothers[channel]);

        // Copy output
        memcpy(channelData, outBuf.data(), buffer.getNumSamples()*sizeof(float));
    }
     
    // Get curves
    {
        std::lock_guard<std::mutex> lock(_curvesMutex);
        
        _displaySinesProcessors[0]->getMagnsBuffer(&_sinesBuffer);
        _displayNoiseProcessors[0]->getMagnsBuffer(&_noiseBuffer);
        _displayOutProcessors[0]->getMagnsBuffer(&_sumBuffer);

        _newBuffersAvailable = true;
    }
}

bool BLSTNAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BLSTNAudioProcessor::createEditor()
{
    return new BLSTNAudioProcessorEditor (*this);
}

void BLSTNAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Create a copy of the parameters state
    juce::ValueTree stateToSave = _parameters.state.createCopy();

    // Add a unified version number for parameters
    constexpr int version = 702; // Unified version number
    stateToSave.setProperty("version", version, nullptr);

    // Serialize the entire state to destData
    juce::MemoryOutputStream stream(destData, true);
    stateToSave.writeToStream(stream);
}

void BLSTNAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Deserialize the state
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    if (auto newState = juce::ValueTree::readFromStream(stream); newState.isValid())
    {
        // Check the version number
        int version = newState.getProperty("version", 0);
        if ((version == 701) || (version == 702))
        {
            // Load the parameter state
            _parameters.state = newState;
        }
        else
        {
            // Handle unknown or future versions
            jassertfalse; // Add migration code or defaults here
        }
    }
}

void
BLSTNAudioProcessor::setSampleRateChangeListener(SampleRateChangeListener listener)
{
    _sampleRateChangeListener = listener;
}

bool
BLSTNAudioProcessor::getBuffers(vector<float> *noiseBuffer,
                                vector<float> *sinesBuffer,
                                vector<float> *sumBuffer)
{
    if (!_newBuffersAvailable)
        return false;
    
    std::lock_guard<std::mutex> lock(_curvesMutex);

    *noiseBuffer = _noiseBuffer;
    *sinesBuffer = _sinesBuffer;
    *sumBuffer = _sumBuffer;

    _newBuffersAvailable = false;

    return true;
}

int
BLSTNAudioProcessor::getLatency()
{
    if (_processors.empty())
        return 0;
    
    int latency = _processors[0]->getLatency();
    
    return latency;
}

void
BLSTNAudioProcessor::setSplitFreq(float freq)
{  
    if (freq >= MIN_SPLIT_FREQ)
    {
        for (int i = 0; i < _bandSplittersIn.size(); i++)
            _bandSplittersIn[i]->setCutoffFreq(0, freq);

        for (int i = 0; i < _bandSplittersOut.size(); i++)
            _bandSplittersOut[i]->setCutoffFreq(0, freq);
    }
}

void
BLSTNAudioProcessor::computeMutes(const bool solos[3], const bool mutes[3], bool resultMutes[3])
{
    for (int i = 0; i < 3; i++)
    {
        resultMutes[i] = mutes[i];

        for (int j = 0; j < 3; j++)
        {
            if (j != i)
            {
                if (solos[j])
                    resultMutes[i] = true;
            }
        }
        
        if (solos[i])
            resultMutes[i] = false;
    }
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE
createPluginFilter()
{
    return new BLSTNAudioProcessor();
}
