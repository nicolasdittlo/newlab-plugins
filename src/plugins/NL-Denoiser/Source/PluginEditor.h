#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

#include "RotarySliderWithValue.h"
#include "BitmapCheckBox.h"
#include "CustomComboBox.h"
#include "PlugNameComponent.h"
#include "HelpButton.h"
#include "SpectrumComponent.h"
#include "SpectrumView.h"
#include "DenoiserSpectrum.h"

class NLDenoiserAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NLDenoiserAudioProcessorEditor (NLDenoiserAudioProcessor&);
    ~NLDenoiserAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void drawVersionText(juce::Graphics& g);

    void handleSampleRateChange(double newSampleRate, int bufferSize);
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NLDenoiserAudioProcessor& _audioProcessor;

    juce::Image _backgroundImage;
    
    std::unique_ptr<RotarySliderWithValue> _ratioSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _ratioAttachment;
    
    std::unique_ptr<RotarySliderWithValue> _thresholdSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _thresholdAttachment;
    
    std::unique_ptr<RotarySliderWithValue> _transBoostSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _transBoostAttachment;
    
    std::unique_ptr<RotarySliderWithValue> _resNoiseThrsSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _resNoiseThrsAttachment;
    
    BitmapCheckBox _learnCheckBox;
    std::unique_ptr<BitmapCheckBoxAttachment> _learnCheckBoxAttachment;
    
    BitmapCheckBox _noiseOnlyCheckBox;
    std::unique_ptr<BitmapCheckBoxAttachment> _noiseOnlyCheckBoxAttachment;
    
    BitmapCheckBox _autoResNoiseCheckBox;
    std::unique_ptr<BitmapCheckBoxAttachment> _autoResNoiseCheckBoxAttachment;
    
    std::unique_ptr<CustomComboBox> _qualityComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> _qualityComboBoxAttachment;
    
    std::unique_ptr<juce::TooltipWindow> _tooltipWindow;

    std::unique_ptr<PlugNameComponent> _plugNameComponent;

    std::unique_ptr<HelpButton> _helpButton;

    std::unique_ptr<SpectrumComponent> _spectrumComponent;

    std::unique_ptr<DenoiserSpectrum> _denoiserSpectrum;
    std::unique_ptr<SpectrumView> _spectrumView;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NLDenoiserAudioProcessorEditor)
};
