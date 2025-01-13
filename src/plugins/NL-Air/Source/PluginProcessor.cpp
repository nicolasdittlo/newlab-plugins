/* Copyright (C) 2025 Nicolas Dittlo <newlab.plugins@gmail.com>
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
#include "AirProcessor.h"
#include "BufProcessor.h"
#include "Utils.h"
#include "ParamSmoother.h"
#include "CrossoverSplitterNBands.h"
#include "Delay.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define OVERLAP 4

#define FFT_SIZE_COEFF 23

#define DEFAULT_TRACKER_THRESHOLD -100.0

#define THRESHOLD_IS_DB 1

#define DEFAULT_SPLIT_FREQ 20.0
#define DEFAULT_SPLIT_FREQ_SMOOTH_TIME_MS 280.0

#define MIN_SPLIT_FREQ 20.0

NLAirAudioProcessor::NLAirAudioProcessor()
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
            juce::ParameterID{"threshold", 700}, "Threshold", -120.0f, 0.0f, -100.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"harmoAirMix", 700}, "Harmo Air Mix", -100.0f, 100.0f, 0.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"outGain", 700}, "Out Gain", -12.0f, 12.0f, 0.0f),
                     std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"smartResynth", 700}, "Smart Resynth", false),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"wetFreq", 700}, "Wet Freq", 20.0f, 20000.0f, 20.0f),
                     std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"wetGain", 700}, "Wet Gain", -12.0f, 12.0f, 0.0f)
                 })
#endif
{
    float sampleRate = 44100.0;
    
    float defaultSplitFreq = DEFAULT_SPLIT_FREQ;
    float splitFreqSmoothTime = DEFAULT_SPLIT_FREQ_SMOOTH_TIME_MS;

    // Adjust, because smoothing is done only once in each processBlock()
    int blockSize = 512;
    splitFreqSmoothTime /= blockSize;
    
    _splitFreqSmoother = new ParamSmoother(sampleRate, defaultSplitFreq,
                                           splitFreqSmoothTime);
}

NLAirAudioProcessor::~NLAirAudioProcessor()
{
    for (int i = 0; i < _overlapAdds.size(); i++)
        delete _overlapAdds[i];

    for (int i = 0; i < _processors.size(); i++)
        delete _processors[i];

    for (int i = 0; i < _outOverlapAdds.size(); i++)
        delete _outOverlapAdds[i];

    for (int i = 0; i < _outProcessors.size(); i++)
        delete _outProcessors[i];

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
NLAirAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool
NLAirAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool
NLAirAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool
NLAirAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double
NLAirAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int
NLAirAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int
NLAirAudioProcessor::getCurrentProgram()
{
    return 0;
}

void
NLAirAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String
NLAirAudioProcessor::getProgramName(int index)
{
    return {};
}

void
NLAirAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void
NLAirAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
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

    // Number of channels changed?
    if (_overlapAdds.size() != numInputChannels)
    {
        // Air
        for (int i = 0; i < _overlapAdds.size(); i++)
            delete _overlapAdds[i];
        _overlapAdds.clear();
        
        for (int i = 0; i < _processors.size(); i++)
            delete _processors[i];
        _processors.clear();

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
    
        for (int i = 0; i < numInputChannels; i++)
        {
            AirProcessor *processor = new AirProcessor(fftSize, OVERLAP, sampleRate);
            processor->setThreshold(DEFAULT_TRACKER_THRESHOLD);

            // For freq splitter
            processor->setEnableSum(false);

            _processors.push_back(processor);
            
            OverlapAdd *overlapAdd = new OverlapAdd(fftSize, OVERLAP, true, true);
            overlapAdd->addProcessor(processor);
            _overlapAdds.push_back(overlapAdd);
        }

        // Out
        for (int i = 0; i < _outOverlapAdds.size(); i++)
            delete _outOverlapAdds[i];
        _outOverlapAdds.clear();
        
        for (int i = 0; i < _outProcessors.size(); i++)
            delete _outProcessors[i];
        _outProcessors.clear();
        
        for (int i = 0; i < numInputChannels; i++)
        {
            BufProcessor *processor = new BufProcessor();
            _outProcessors.push_back(processor);
            
            OverlapAdd *overlapAdd = new OverlapAdd(fftSize, OVERLAP, true, false);
            overlapAdd->addProcessor(processor);
            _outOverlapAdds.push_back(overlapAdd);
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

    // Air
    for (int i = 0; i < _overlapAdds.size(); i++)
    {
        _overlapAdds[i]->setFftSize(fftSize);
        _overlapAdds[i]->setOverlap(OVERLAP);
    }

    for (int i = 0; i < _processors.size(); i++)
        _processors[i]->reset(fftSize, OVERLAP, sampleRate);

    // Out
    for (int i = 0; i < _outOverlapAdds.size(); i++)
    {
        _outOverlapAdds[i]->setFftSize(fftSize);
        _outOverlapAdds[i]->setOverlap(OVERLAP);
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
    int latency = getLatency(samplesPerBlock);
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
NLAirAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool
NLAirAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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
NLAirAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    auto threshold = _parameters.getRawParameterValue("threshold")->load();
    auto harmoAirMix = _parameters.getRawParameterValue("harmoAirMix")->load();
    auto outGain = _parameters.getRawParameterValue("outGain")->load();
    auto smartResynth = _parameters.getRawParameterValue("smartResynth")->load();
    auto wetFreq = _parameters.getRawParameterValue("wetFreq")->load();
    auto wetGain = _parameters.getRawParameterValue("wetGain")->load();
    
    harmoAirMix *= 0.01;
    harmoAirMix = -harmoAirMix;
    outGain = Utils::DBToAmp(outGain);

    bool smartResynthChanged = (smartResynth > 0.5) != _prevSmartResynthParam;
    _prevSmartResynthParam = (smartResynth > 0.5);

    wetGain = Utils::DBToAmp(wetGain);
    
    // Set parameters
    for (int i = 0; i < _processors.size(); i++)
    {
        _processors[i]->setThreshold(threshold);
        _processors[i]->setMix(harmoAirMix);
        _processors[i]->setUseSoftMasks(smartResynth > 0.5);
    }

    if (smartResynthChanged)
    {            
        // Update latency
        int latency = getLatency(buffer.getNumSamples());
        setLatencySamples(latency);
        updateHostDisplay();
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
        
        _overlapAdds[channel]->feed(inBuf);
        
        vector<float> outBuf;
        int numSamplesToFlush = _overlapAdds[channel]->getOutSamples(&outBuf, buffer.getNumSamples());
        _overlapAdds[channel]->flushOutSamples(numSamplesToFlush);

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
        _outOverlapAdds[channel]->feed(outBuf);

        // Apply out gain
        Utils::applyGain(outBuf, &outBuf, _outGainSmoothers[channel]);

        // Copy output
        memcpy(channelData, outBuf.data(), buffer.getNumSamples()*sizeof(float));
    }
     
    // Get curves
    {
        std::lock_guard<std::mutex> lock(_curvesMutex);

        _processors[0]->getNoiseBuffer(&_airBuffer);
        _processors[0]->getHarmoBuffer(&_harmoBuffer);

        _outProcessors[0]->getMagnsBuffer(&_sumBuffer);

        _newBuffersAvailable = true;
    }
}

bool NLAirAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NLAirAudioProcessor::createEditor()
{
    return new NLAirAudioProcessorEditor (*this);
}

void NLAirAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Create a copy of the parameters state
    juce::ValueTree stateToSave = _parameters.state.createCopy();

    // Add a unified version number for parameters and noise profile
    constexpr int version = 700; // Unified version number
    stateToSave.setProperty("version", version, nullptr);

    // Serialize the entire state to destData
    juce::MemoryOutputStream stream(destData, true);
    stateToSave.writeToStream(stream);
}

void NLAirAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Deserialize the state
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    if (auto newState = juce::ValueTree::readFromStream(stream); newState.isValid())
    {
        // Check the version number
        int version = newState.getProperty("version", 0);
        if (version == 700)
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
NLAirAudioProcessor::setSampleRateChangeListener(SampleRateChangeListener listener)
{
    _sampleRateChangeListener = listener;
}

bool
NLAirAudioProcessor::getBuffers(vector<float> *airBuffer,
                                vector<float> *harmoBuffer,
                                vector<float> *sumBuffer)
{
    if (!_newBuffersAvailable)
        return false;
    
    std::lock_guard<std::mutex> lock(_curvesMutex);

    *airBuffer = _airBuffer;
    *harmoBuffer = _harmoBuffer;
    *sumBuffer = _sumBuffer;

    _newBuffersAvailable = false;

    return true;
}

int
NLAirAudioProcessor::getLatency(int blockSize)
{
    if (_processors.empty())
        return 0;
    
    int fftSize = Utils::nearestPowerOfTwo(_sampleRate/FFT_SIZE_COEFF);
    int hopSize = fftSize/OVERLAP;
    
    int latency = fftSize - hopSize;

    if (blockSize < hopSize)
        latency += hopSize - blockSize;

    int processorLatency = _processors[0]->getLatency();
    latency += processorLatency;

    return latency;
}

void
NLAirAudioProcessor::setSplitFreq(float freq)
{  
    if (freq >= MIN_SPLIT_FREQ)
    {
        for (int i = 0; i < _bandSplittersIn.size(); i++)
            _bandSplittersIn[i]->setCutoffFreq(0, freq);

        for (int i = 0; i < _bandSplittersOut.size(); i++)
                _bandSplittersOut[i]->setCutoffFreq(0, freq);
    }
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE
createPluginFilter()
{
    return new NLAirAudioProcessor();
}
