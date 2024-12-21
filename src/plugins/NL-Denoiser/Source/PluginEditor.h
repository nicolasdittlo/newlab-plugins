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
#include <CustomComboBox.h>

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
    NLDenoiserAudioProcessor& _audioProcessor;

    juce::Image _backgroundImage;
    
    std::unique_ptr<RotarySliderWithValue> _ratioSlider;
    std::unique_ptr<RotarySliderWithValue> _thresholdSlider;
    std::unique_ptr<RotarySliderWithValue> _transBoostSlider;
    std::unique_ptr<RotarySliderWithValue> _resNoiseThrsSlider;

    BitmapCheckBox _learnCheckBox;
    BitmapCheckBox _noiseOnlyCheckBox;
    BitmapCheckBox _autoResNoiseCheckBox;

    std::unique_ptr<CustomComboBox> _qualityComboBox;

    std::unique_ptr<juce::TooltipWindow> _tooltipWindow;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NLDenoiserAudioProcessorEditor)
};
