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

#pragma once

#include <vector>
using namespace std;

#include <JuceHeader.h>

class OverlapAdd;
class AirProcessor;
class TransientShaperProcessor;
class NLAirAudioProcessor  : public juce::AudioProcessor
{
public:
    using SampleRateChangeListener = std::function<void(double, int)>;
    
    NLAirAudioProcessor();
    ~NLAirAudioProcessor() override;

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

    bool getBuffers(vector<float> *signalBuffer,
                    vector<float> *noiseBuffer,
                    vector<float> *noiseProfileBuffer);
    
public:
    juce::AudioProcessorValueTreeState _parameters;
    
private:
    int getLatency(int blockSize);
    
    vector<OverlapAdd *> _overlapAdds;
    vector<AirProcessor *> _processors;
    vector<TransientShaperProcessor *> _transientProcessors;
    
    bool _prevSmartResynthParam = false;
    
    double _sampleRate = 0.0;
    SampleRateChangeListener _sampleRateChangeListener = nullptr;

    vector<float> _signalBuffer;
    vector<float> _noiseBuffer;
    vector<float> _noiseProfileBuffer;
    
    std::mutex _curvesMutex;
    bool _newBuffersAvailble = false;

    vector<vector<float> > _nativeNoiseProfiles;
    bool _mustSetNativeNoiseProfiles = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NLAirAudioProcessor)
};
