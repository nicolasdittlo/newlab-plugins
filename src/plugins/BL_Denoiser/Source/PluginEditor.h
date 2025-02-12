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

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

#include "RotarySliderWithValue.h"
#include "BitmapCheckBox.h"
#include "CustomComboBox.h"
#include "PlugNameComponent.h"
#include "HelpButton.h"
#include "SpectrumComponentGL.h"
#include "SpectrumComponentJuce.h"
#include "SpectrumViewNVG.h"
#include "SpectrumViewJuce.h"
#include "DenoiserSpectrum.h"

class BLDenoiserAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    BLDenoiserAudioProcessorEditor (BLDenoiserAudioProcessor&);
    ~BLDenoiserAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setScaleFactor(float newScale) override;
    
private:
    void drawVersionText(juce::Graphics& g);

    void handleSampleRateChange(double newSampleRate, int bufferSize);

    void timerCallback() override;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BLDenoiserAudioProcessor& _audioProcessor;

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

    std::unique_ptr<DenoiserSpectrum> _denoiserSpectrum = nullptr;
    
#ifndef __arm64__
    std::unique_ptr<SpectrumComponentGL> _spectrumComponent;
    std::unique_ptr<SpectrumViewNVG> _spectrumView;
#else
    std::unique_ptr<SpectrumComponentJuce> _spectrumComponent;
    std::unique_ptr<SpectrumViewJuce> _spectrumView;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BLDenoiserAudioProcessorEditor)
};
