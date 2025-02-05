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
#include "AirSpectrum.h"

class BLAirAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    BLAirAudioProcessorEditor (BLAirAudioProcessor&);
    ~BLAirAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setScaleFactor(float newScale) override;

private:
    void drawVersionText(juce::Graphics& g);

    void handleSampleRateChange(double newSampleRate, int bufferSize);

    void timerCallback() override;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BLAirAudioProcessor& _audioProcessor;

    juce::Image _backgroundImage;
    
    std::unique_ptr<RotarySliderWithValue> _thresholdSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _thresholdAttachment;
    
    std::unique_ptr<RotarySliderWithValue> _harmoAirMixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _harmoAirMixAttachment;
    
    std::unique_ptr<RotarySliderWithValue> _outGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _outGainAttachment;

    BitmapCheckBox _smartResynthCheckBox;
    std::unique_ptr<BitmapCheckBoxAttachment> _smartResynthCheckBoxAttachment;

    std::unique_ptr<RotarySliderWithValue> _wetFreqSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _wetFreqAttachment;

    std::unique_ptr<RotarySliderWithValue> _wetGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _wetGainAttachment;
    
    std::unique_ptr<juce::TooltipWindow> _tooltipWindow;

    std::unique_ptr<PlugNameComponent> _plugNameComponent;

    std::unique_ptr<HelpButton> _helpButton;

#ifndef __APPLE__
    std::unique_ptr<SpectrumComponentGL> _spectrumComponent;
    std::unique_ptr<SpectrumViewNVG> _spectrumView;
#else
    std::unique_ptr<SpectrumComponentJuce> _spectrumComponent;
    std::unique_ptr<SpectrumViewJuce> _spectrumView;
#endif
    
    std::unique_ptr<AirSpectrum> _airSpectrum = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BLAirAudioProcessorEditor)
};
