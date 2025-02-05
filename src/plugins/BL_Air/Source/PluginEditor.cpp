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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <Config.h>
#include <CustomLookAndFeel.h>
#include <VersionTextDrawer.h>
#include <ManualPdfViewer.h>
#include <DemoTextDrawer.h>
#include <AirProcessor.h>

#define VERSION_STR "7.0.0"

#define PLUGIN_WIDTH 456
#define PLUGIN_HEIGHT 520

BLAirAudioProcessorEditor::BLAirAudioProcessorEditor(BLAirAudioProcessor& p)
    : AudioProcessorEditor(&p), _audioProcessor(p)
{    
    // Set the custom look and feel
    juce::LookAndFeel::setDefaultLookAndFeel(new CustomLookAndFeel());
    
    // Load the background image from binary resources
    _backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);

    // Configure the threshold slider with units
    _thresholdSlider = std::make_unique<RotarySliderWithValue>("", "dB", SliderSize::SmallSlider);
    _thresholdSlider->setRange(-120.0, 0.0, 0.1);
    _thresholdSlider->setDefaultValue(-100.0);
    _thresholdSlider->setTooltip("Threshold - Harmonic detection threshold");
    _thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "threshold", _thresholdSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_thresholdSlider);

    // Configure the harmo air mix slider with units
    _harmoAirMixSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::BigSlider);
    _harmoAirMixSlider->setRange(-100.0, 100.0, 0.1);
    _harmoAirMixSlider->setDefaultValue(0.0);
    _harmoAirMixSlider->setTooltip("Harmonic/Air - Mix between harmonic and air");
    _harmoAirMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "harmoAirMix", _harmoAirMixSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_harmoAirMixSlider);

    // Configure the out gain slider with units
    _outGainSlider = std::make_unique<RotarySliderWithValue>("", "dB", SliderSize::SmallSlider);
    _outGainSlider->setRange(-12.0, 12.0, 0.1);
    _outGainSlider->setDefaultValue(0.0);
    _outGainSlider->setTooltip("Out Gain - Output gain");
    _outGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "outGain", _outGainSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_outGainSlider);

    // smart resynth check box
    _smartResynthCheckBox.setTooltip("Smart Resynthesis - Higher quality resynthesis");

    _smartResynthCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
        (_audioProcessor._parameters, "smartResynth", _smartResynthCheckBox);

    // Add the smart resynth check box to the editor
    addAndMakeVisible(_smartResynthCheckBox);

    // Configure the wet freq slider with units
    _wetFreqSlider = std::make_unique<RotarySliderWithValue>("", "Hz", SliderSize::SmallSlider);
    _wetFreqSlider->setRange(20.0, 20000.0, 1.0);
    _wetFreqSlider->setDefaultValue(20.0);
    _wetFreqSlider->setParamShape(0.25);
    _wetFreqSlider->setTooltip("Wet Limit Frequency - Signal is untouched before");
    _wetFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "wetFreq", _wetFreqSlider->getSlider());

    // Add the rotary slider to the editor
    addAndMakeVisible(*_wetFreqSlider);

    // Configure the wet gain slider with units
    _wetGainSlider = std::make_unique<RotarySliderWithValue>("", "dB", SliderSize::SmallSlider);
    _wetGainSlider->setRange(-12.0, 12.0, 0.1);
    _wetGainSlider->setDefaultValue(0.0);
    _wetGainSlider->setTooltip("Wet Gain - Gain applied to wet signal");
    _wetGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "wetGain", _wetGainSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_wetGainSlider);

    // Tooltip window
    _tooltipWindow = std::make_unique<juce::TooltipWindow>(this, 500);

    // Plugin name
    _plugNameComponent = std::make_unique<PlugNameComponent>();
    addAndMakeVisible(*_plugNameComponent);

    // Help button
    _helpButton = std::make_unique<HelpButton>();

    _helpButton->setTooltip("Help - Display Help");
    
    _helpButton->onStateChange = [] () { ManualPdfViewer::openEmbeddedPdf(); };
    addAndMakeVisible(*_helpButton);

#ifndef __APPLE__
    _spectrumComponent = std::make_unique<SpectrumComponentGL>();
#else
    _spectrumComponent = std::make_unique<SpectrumComponentJuce>();
#endif
    addAndMakeVisible(*_spectrumComponent);

#ifndef __APPLE__
    _spectrumView = std::make_unique<SpectrumViewNVG>();
#else
    _spectrumView = std::make_unique<SpectrumViewJuce>();
#endif
    _airSpectrum = std::make_unique<AirSpectrum>(_spectrumView.get(), 44100.0, 2048);

    _spectrumComponent->setSpectrumView(_spectrumView.get());
    
    // Set the editor's size
    setSize(PLUGIN_WIDTH, PLUGIN_HEIGHT);

    // Register the sample rate change listener
    _audioProcessor.setSampleRateChangeListener([this](double sampleRate, int bufferSize)
    {
        juce::MessageManager::callAsync([this, sampleRate, bufferSize]()
        {
            handleSampleRateChange(sampleRate, bufferSize);
        });
    });
    
    startTimerHz(30);
}

BLAirAudioProcessorEditor::~BLAirAudioProcessorEditor()
{
    _audioProcessor.setSampleRateChangeListener(nullptr);
        
    stopTimer();
    
    // Reset the LookAndFeel to avoid dangling references
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void BLAirAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Clear the background with a default color
    g.fillAll(juce::Colours::black);

    // Draw the background image
    if (_backgroundImage.isValid())
        g.drawImageAt(_backgroundImage, 0, 0);

    // Call the function to draw the version text
    VersionTextDrawer::drawVersionText(*this, g, VERSION_STR);

    // Please be kind, do not modify this code and share binaries
#if DEMO_VERSION
    DemoTextDrawer::drawDemoText(*this, g, "DEMO");
#endif
}

void BLAirAudioProcessorEditor::resized()
{
    if ((getWidth() != PLUGIN_WIDTH) || (getHeight() != PLUGIN_HEIGHT))
    {
        setSize(PLUGIN_WIDTH, PLUGIN_HEIGHT);
        return;
    }

    auto smallSliderWidth = 72; // Updated width to match the label width for small sliders
    auto smallSliderHeight = 36 + 25 + 20; // 36 for slider, 25 for spacing, 20 for label height

    _thresholdSlider->setBounds(60 - (smallSliderWidth - 36) / 2, // Center the slider
                                374,
                                smallSliderWidth,
                                smallSliderHeight);

    auto bigSliderWidth = 72;
    auto bigSliderHeight = 72 + 25 + 20; // 72 for slider, 25 for spacing, 20 for label height
    _harmoAirMixSlider->setBounds(150, 338, bigSliderWidth, bigSliderHeight);
    
    _outGainSlider->setBounds(360 - (smallSliderWidth - 36) / 2, // Center the slider
                              274,
                              smallSliderWidth,
                              smallSliderHeight);

    _smartResynthCheckBox.setBounds(40, 282, 20, 20);

    _wetFreqSlider->setBounds(270 - (smallSliderWidth - 36) / 2, // Center the slider
                              374,
                              smallSliderWidth,
                              smallSliderHeight);

    _wetGainSlider->setBounds(360 - (smallSliderWidth - 36) / 2, // Center the slider
                              374,
                              smallSliderWidth,
                              smallSliderHeight);

    _plugNameComponent->setBounds(getWidth()/2 - _plugNameComponent->getWidth()/2,
                                  getHeight() - _plugNameComponent->getHeight() - 15.0,
                                  _plugNameComponent->getWidth(),
                                  _plugNameComponent->getHeight());

    _helpButton->setBounds(getWidth() - 20 - 14, getHeight() - 20 - 10, 20, 20);

    _spectrumComponent->setBounds(0, 0, 456, 236);
}

void 
BLAirAudioProcessorEditor::setScaleFactor(float newScale)
{
    // Do nothing to prevent host scaling
}

void
BLAirAudioProcessorEditor::handleSampleRateChange(double sampleRate, int bufferSize)
{
    if (_airSpectrum != nullptr)
        _airSpectrum->reset(bufferSize, sampleRate);
}

void
BLAirAudioProcessorEditor::timerCallback()
{
    vector<float> noiseBuffer;
    vector<float> harmoBuffer;
    vector<float> sumBuffer;
        
    bool newBuffersAvailable = _audioProcessor.getBuffers(&noiseBuffer,
                                                          &harmoBuffer,
                                                          &sumBuffer);

    if (newBuffersAvailable)
    {
        _airSpectrum->updateCurves(noiseBuffer,
                                   harmoBuffer,
                                   sumBuffer);
    }

    auto harmoAirMix = _audioProcessor._parameters.getRawParameterValue("harmoAirMix")->load();
    harmoAirMix *= 0.01;
    harmoAirMix = -harmoAirMix;
    _airSpectrum->setMix(harmoAirMix);
        
#ifdef __linux__
    _spectrumComponent->repaint();
#endif
#ifdef __APPLE__
    _spectrumComponent->repaint();
#endif
}
