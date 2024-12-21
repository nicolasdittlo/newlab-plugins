/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include <RotarySliderWithValue.h>
#include <BitmapCheckBox.h>

class NLDenoiserAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NLDenoiserAudioProcessorEditor (NLDenoiserAudioProcessor&);
    ~NLDenoiserAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NLDenoiserAudioProcessor& audioProcessor;

    juce::Image backgroundImage;
    
    std::unique_ptr<RotarySliderWithValue> ratioSlider;
    std::unique_ptr<RotarySliderWithValue> thresholdSlider;
    std::unique_ptr<RotarySliderWithValue> transBoostSlider;
    std::unique_ptr<RotarySliderWithValue> resNoiseThrsSlider;

    BitmapCheckBox learnCheckBox;
    BitmapCheckBox noiseOnlyCheckBox;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NLDenoiserAudioProcessorEditor)
};
