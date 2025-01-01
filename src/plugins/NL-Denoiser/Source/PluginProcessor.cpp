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

#include <OverlapAdd.h>
#include <DenoiserProcessor.h>
#include <TransientShaperProcessor.h>

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define OVERLAP_0 4
#define OVERLAP_1 8
#define OVERLAP_2 16
#define OVERLAP_3 32

// Half amp, half freq
#define TRANSIENT_FREQ_AMP_RATIO 0.5

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
            juce::ParameterID{"threshold", 700}, "Threshold", 0.0f, 100.0f, 50.0f),
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

    for (int i = 0; i < _transientProcessors.size(); i++)
        delete _transientProcessors[i];
}

const juce::String
NLDenoiserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool
NLDenoiserAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool
NLDenoiserAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool
NLDenoiserAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double
NLDenoiserAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int
NLDenoiserAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int
NLDenoiserAudioProcessor::getCurrentProgram()
{
    return 0;
}

void
NLDenoiserAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String
NLDenoiserAudioProcessor::getProgramName(int index)
{
    return {};
}

void
NLDenoiserAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void
NLDenoiserAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    int numInputChannels = getTotalNumInputChannels();
    
    int fftSize = juce::nextPowerOfTwo(sampleRate/FFT_SIZE_COEFF);

    if (sampleRate != _sampleRate)
    {
        _sampleRate = sampleRate;
        
        // Notify listener
        if (_sampleRateChangeListener)
            _sampleRateChangeListener(sampleRate, fftSize/2 + 1);
    }
    
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

        for (int i = 0; i < _transientProcessors.size(); i++)
            delete _transientProcessors[i];
        _transientProcessors.clear();
        
        for (int i = 0; i < numInputChannels; i++)
        {
            DenoiserProcessor *processor = new DenoiserProcessor(fftSize/2 + 1, overlap, threshold);
            _processors.push_back(processor);

            TransientShaperProcessor *transientProcessor = new TransientShaperProcessor(sampleRate);
            _transientProcessors.push_back(transientProcessor);
            
            OverlapAdd *overlapAdd = new OverlapAdd(fftSize, overlap, true, true);
            overlapAdd->addProcessor(processor);
            overlapAdd->addProcessor(transientProcessor);
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

    for (int i = 0; i < _transientProcessors.size(); i++)
        _transientProcessors[i]->reset(sampleRate);
    
    // Update latency
    int latency = getLatency(samplesPerBlock);
    setLatencySamples(latency);
    updateHostDisplay();
}

void
NLDenoiserAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool
NLDenoiserAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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
NLDenoiserAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    auto ratio = _parameters.getRawParameterValue("ratio")->load();
    auto threshold = _parameters.getRawParameterValue("threshold")->load();
    auto transientBoost = _parameters.getRawParameterValue("transientBoost")->load();
    auto residualNoise = _parameters.getRawParameterValue("residualNoise")->load();
    auto learnMode = _parameters.getRawParameterValue("learnModeParamID")->load();
    auto noiseOnly = _parameters.getRawParameterValue("noiseOnlyParamID")->load();
    auto softDenoise = _parameters.getRawParameterValue("softDenoiseParamID")->load();
    auto quality = _parameters.getRawParameterValue("quality")->load();
    
    ratio *= 0.01;
    threshold *= 0.01;
    transientBoost *= 0.01;
    residualNoise *= 0.01;
    
    bool qualityChanged = (quality != _prevQualityParam);
    _prevQualityParam = quality;

    int overlap = getOverlap(quality);

    bool softDenoiseChanged = (softDenoise > 0.5) != _prevSoftDenoiseParam;
    _prevSoftDenoiseParam = (softDenoise > 0.5);
    
    // Set parameters
    for (int i = 0; i < _processors.size(); i++)
    {
        _processors[i]->setThreshold(threshold);
        _processors[i]->setResNoiseThrs(residualNoise);
        _processors[i]->setBuildingNoiseStatistics(learnMode);
        _processors[i]->setAutoResNoise(softDenoise);
        _processors[i]->setRatio(ratio);
        _processors[i]->setNoiseOnly((noiseOnly > 0.5));
        
        if (qualityChanged)
        {            
            _processors[i]->setOverlap(overlap);
            _overlapAdds[i]->setOverlap(overlap);
        }
    }

    for (int i = 0; i < _transientProcessors.size(); i++)
    {
        _transientProcessors[i]->setFreqAmpRatio(TRANSIENT_FREQ_AMP_RATIO);
        _transientProcessors[i]->setSoftHard(transientBoost);
    }
    
    if (qualityChanged || softDenoiseChanged)
    {            
        // Update latency
        int latency = getLatency(buffer.getNumSamples());
        setLatencySamples(latency);
        updateHostDisplay();
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

        memcpy(channelData, outBuf.data(), buffer.getNumSamples()*sizeof(float));
    }
    
    // Get curves
    {
        std::lock_guard<std::mutex> lock(_curvesMutex);
        
        _processors[0]->getSignalBuffer(&_signalBuffer);
        _processors[0]->getNoiseBuffer(&_noiseBuffer);
        _processors[0]->getNoiseCurve(&_noiseProfileBuffer);
        
        _newBuffersAvailble = true;
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
    // Create a copy of the parameters state
    juce::ValueTree stateToSave = _parameters.state.createCopy();

    // Add a unified version number for parameters and noise profile
    constexpr int version = 700; // Unified version number
    stateToSave.setProperty("version", version, nullptr);

    vector<vector< float> > noiseProfileArray;
    noiseProfileArray.resize(_processors.size());
    for (int i = 0; i < _processors.size(); i++)
        _processors[i]->getNativeNoiseCurve(&noiseProfileArray[i]);
    
    // Serialize the noise profile array into a MemoryBlock
    juce::MemoryBlock noiseProfileBlock;
    {
        juce::MemoryOutputStream noiseStream(noiseProfileBlock, true);

        // Write the number of vectors
        noiseStream.writeInt(static_cast<int>(noiseProfileArray.size()));

        // Write each vector
        for (const auto& vector : noiseProfileArray)
        {
            noiseStream.writeInt(static_cast<int>(vector.size()));
            for (float value : vector)
                noiseStream.writeFloat(value);
        }
    }

    // Convert the noise profile binary data to a Base64 string and save it
    stateToSave.setProperty("noiseProfile", noiseProfileBlock.toBase64Encoding(), nullptr);

    // Serialize the entire state to destData
    juce::MemoryOutputStream stream(destData, true);
    stateToSave.writeToStream(stream);
}

void NLDenoiserAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
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

            // Restore the noise profile from the binary blob
            if (newState.hasProperty("noiseProfile"))
            {
                vector<vector<float> > noiseProfileArray;
                
                juce::String encodedBlob = newState["noiseProfile"].toString();
                juce::MemoryBlock noiseProfileBlock;
                if (noiseProfileBlock.fromBase64Encoding(encodedBlob))
                {
                    juce::MemoryInputStream noiseStream(noiseProfileBlock, false);

                    // Clear existing data
                    noiseProfileArray.clear();

                    // Read the number of vectors
                    int numVectors = noiseStream.readInt();

                    // Read each vector
                    for (int i = 0; i < numVectors; ++i)
                    {
                        int vectorSize = noiseStream.readInt();
                        std::vector<float> vector;

                        for (int j = 0; j < vectorSize; ++j)
                            vector.push_back(noiseStream.readFloat());

                        noiseProfileArray.push_back(std::move(vector));
                    }
                }

                for (int i = 0; i < _processors.size(); i++)
                {
                    if (i < noiseProfileArray.size())
                        _processors[i]->setNativeNoiseCurve(noiseProfileArray[i]);
                }
            }
        }
        else
        {
            // Handle unknown or future versions
            jassertfalse; // Add migration code or defaults here
        }
    }
}

void
NLDenoiserAudioProcessor::setSampleRateChangeListener(SampleRateChangeListener listener)
{
    _sampleRateChangeListener = listener;
}

bool
NLDenoiserAudioProcessor::getBuffers(vector<float> *signalBuffer,
                                     vector<float> *noiseBuffer,
                                     vector<float> *noiseProfileBuffer)
{
    if (!_newBuffersAvailble)
        return false;
    
    std::lock_guard<std::mutex> lock(_curvesMutex);

    *signalBuffer = _signalBuffer;
    *noiseBuffer = _noiseBuffer;
    *noiseProfileBuffer = _noiseProfileBuffer;

    _newBuffersAvailble = false;

    return true;
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

int
NLDenoiserAudioProcessor::getLatency(int blockSize)
{
    if (_processors.empty())
        return 0;
    
    int fftSize = juce::nextPowerOfTwo(_sampleRate/FFT_SIZE_COEFF);
    auto quality = _parameters.getRawParameterValue("quality")->load();
    int overlap = getOverlap(quality);
    int hopSize = fftSize/overlap;
    
    int latency = fftSize - hopSize;

    if (blockSize < hopSize)
        latency += hopSize - blockSize;

    int processorLatency = _processors[0]->getLatency();
    latency += processorLatency;

    return latency;
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE
createPluginFilter()
{
    return new NLDenoiserAudioProcessor();
}
