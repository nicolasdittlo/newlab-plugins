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
#include <STNProcessor.h>

#define VERSION_STR "7.0.1"

#define PLUGIN_WIDTH 464
#define PLUGIN_HEIGHT 553

BLSTNAudioProcessorEditor::BLSTNAudioProcessorEditor(BLSTNAudioProcessor& p)
: AudioProcessorEditor(&p), _audioProcessor(p),
  _soloSinesCheckBox(BinaryData::solo_unchecked_png, BinaryData::solo_unchecked_pngSize,
                     BinaryData::solo_checked_png, BinaryData::solo_checked_pngSize),
  _muteSinesCheckBox(BinaryData::mute_unchecked_png, BinaryData::mute_unchecked_pngSize,
                     BinaryData::mute_checked_png, BinaryData::mute_checked_pngSize),
  _soloTransientsCheckBox(BinaryData::solo_unchecked_png, BinaryData::solo_unchecked_pngSize,
                          BinaryData::solo_checked_png, BinaryData::solo_checked_pngSize),
  _muteTransientsCheckBox(BinaryData::mute_unchecked_png, BinaryData::mute_unchecked_pngSize,
                          BinaryData::mute_checked_png, BinaryData::mute_checked_pngSize),
  _soloNoiseCheckBox(BinaryData::solo_unchecked_png, BinaryData::solo_unchecked_pngSize,
                     BinaryData::solo_checked_png, BinaryData::solo_checked_pngSize),
  _muteNoiseCheckBox(BinaryData::mute_unchecked_png, BinaryData::mute_unchecked_pngSize,
                     BinaryData::mute_checked_png, BinaryData::mute_checked_pngSize)
                          
{    
    // Set the custom look and feel
    juce::LookAndFeel::setDefaultLookAndFeel(new CustomLookAndFeel());
    
    // Load the background image from binary resources
    _backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);

    // Configure the mix sines slider with units
    _mixSinesSlider = std::make_unique<RotarySliderWithValue>("", "dB", SliderSize::BigSlider);
    _mixSinesSlider->setRange(-12.0, 12.0, 0.1);
    _mixSinesSlider->setDefaultValue(0.0);
    _mixSinesSlider->setTooltip("Sines - Mix sines");
    _mixSinesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "sinesMix", _mixSinesSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_mixSinesSlider);

    // Configure the mix transients slider with units
    _mixTransientsSlider = std::make_unique<RotarySliderWithValue>("", "dB", SliderSize::BigSlider);
    _mixTransientsSlider->setRange(-12.0, 12.0, 0.1);
    _mixTransientsSlider->setDefaultValue(0.0);
    _mixTransientsSlider->setTooltip("Transients - Mix transients");
    _mixTransientsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "transientsMix", _mixTransientsSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_mixTransientsSlider);

    // Configure the mix noise slider with units
    _mixNoiseSlider = std::make_unique<RotarySliderWithValue>("", "dB", SliderSize::BigSlider);
    _mixNoiseSlider->setRange(-12.0, 12.0, 0.1);
    _mixNoiseSlider->setDefaultValue(0.0);
    _mixNoiseSlider->setTooltip("Noise - Mix noise");
    _mixNoiseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "noiseMix", _mixNoiseSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_mixNoiseSlider);
    
    // Configure the out gain slider with units
    _outGainSlider = std::make_unique<RotarySliderWithValue>("", "dB", SliderSize::SmallSlider);
    _outGainSlider->setRange(-12.0, 12.0, 0.1);
    _outGainSlider->setDefaultValue(0.0);
    _outGainSlider->setTooltip("Out Gain - Output gain");
    _outGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "outGain", _outGainSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_outGainSlider);

    // solo sines check box
    _soloSinesCheckBox.setTooltip("Solo sines");

    _soloSinesCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
        (_audioProcessor._parameters, "soloSines", _soloSinesCheckBox);

    // Add the solo sines check box to the editor
    addAndMakeVisible(_soloSinesCheckBox);

    // mute sines check box
    _muteSinesCheckBox.setTooltip("Mute sines");

    _muteSinesCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
        (_audioProcessor._parameters, "muteSines", _muteSinesCheckBox);

    // Add the mute sines check box to the editor
    addAndMakeVisible(_muteSinesCheckBox);

    // solo transients check box
    _soloTransientsCheckBox.setTooltip("Solo transients");

    _soloTransientsCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
        (_audioProcessor._parameters, "soloTransients", _soloTransientsCheckBox);

    // Add the solo transients check box to the editor
    addAndMakeVisible(_soloTransientsCheckBox);

    // mute transients check box
    _muteTransientsCheckBox.setTooltip("Mute transients");

    _muteTransientsCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
        (_audioProcessor._parameters, "muteTransients", _muteTransientsCheckBox);

    // Add the mute transients check box to the editor
    addAndMakeVisible(_muteTransientsCheckBox);

    // solo noise check box
    _soloNoiseCheckBox.setTooltip("Solo noise");

    _soloNoiseCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
        (_audioProcessor._parameters, "soloNoise", _soloNoiseCheckBox);

    // Add the solo noise check box to the editor
    addAndMakeVisible(_soloNoiseCheckBox);

    // mute noise check box
    _muteNoiseCheckBox.setTooltip("Mute noise");

    _muteNoiseCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
        (_audioProcessor._parameters, "muteNoise", _muteNoiseCheckBox);

    // Add the mute noise check box to the editor
    addAndMakeVisible(_muteNoiseCheckBox);
    
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

#if RENDER_GL
    _spectrumComponent = std::make_unique<SpectrumComponentGL>();
#else
    _spectrumComponent = std::make_unique<SpectrumComponentJuce>();
#endif
    addAndMakeVisible(*_spectrumComponent);

#if RENDER_GL
    _spectrumView = std::make_unique<SpectrumViewNVG>();
#else
    _spectrumView = std::make_unique<SpectrumViewJuce>();
#endif
    _stnSpectrum = std::make_unique<STNSpectrum>(_spectrumView.get(), 44100.0, 2048);

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

BLSTNAudioProcessorEditor::~BLSTNAudioProcessorEditor()
{
    _audioProcessor.setSampleRateChangeListener(nullptr);
        
    stopTimer();
    
    // Reset the LookAndFeel to avoid dangling references
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void BLSTNAudioProcessorEditor::paint(juce::Graphics& g)
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

void BLSTNAudioProcessorEditor::resized()
{
    if ((getWidth() != PLUGIN_WIDTH) || (getHeight() != PLUGIN_HEIGHT))
    {
        setSize(PLUGIN_WIDTH, PLUGIN_HEIGHT);
        return;
    }

    auto bigSliderWidth = 72;
    auto bigSliderHeight = 72 + 25 + 20; // 72 for slider, 25 for spacing, 20 for label height
    _mixSinesSlider->setBounds(48, 230, bigSliderWidth, bigSliderHeight);

    _mixTransientsSlider->setBounds(196, 230, bigSliderWidth, bigSliderHeight);

    _mixNoiseSlider->setBounds(342, 230, bigSliderWidth, bigSliderHeight);

    auto smallSliderWidth = 72; // Updated width to match the label width for small sliders
    auto smallSliderHeight = 36 + 25 + 20; // 36 for slider, 25 for spacing, 20 for label height
    
    _outGainSlider->setBounds(360 - (smallSliderWidth - 36) / 2, // Center the slider
                              406,
                              smallSliderWidth,
                              smallSliderHeight);
    
    _wetFreqSlider->setBounds(67 - (smallSliderWidth - 36) / 2, // Center the slider
                              406,
                              smallSliderWidth,
                              smallSliderHeight);

    _wetGainSlider->setBounds(214 - (smallSliderWidth - 36) / 2, // Center the slider
                              406,
                              smallSliderWidth,
                              smallSliderHeight);

    _soloSinesCheckBox.setBounds(61, 358, 20, 20);
    _muteSinesCheckBox.setBounds(89, 358, 20, 20);

    _soloTransientsCheckBox.setBounds(208, 358, 20, 20);
    _muteTransientsCheckBox.setBounds(236, 358, 20, 20);

    _soloNoiseCheckBox.setBounds(355, 358, 20, 20);
    _muteNoiseCheckBox.setBounds(383, 358, 20, 20);
    
    _plugNameComponent->setBounds(getWidth()/2 - _plugNameComponent->getWidth()/2,
                                  getHeight() - _plugNameComponent->getHeight() - 15.0,
                                  _plugNameComponent->getWidth(),
                                  _plugNameComponent->getHeight());

    _helpButton->setBounds(getWidth() - 20 - 14, getHeight() - 20 - 10, 20, 20);

    _spectrumComponent->setBounds(0, 0, 464, 198);
}

void 
BLSTNAudioProcessorEditor::setScaleFactor(float newScale)
{
    // Do nothing to prevent host scaling
}

void
BLSTNAudioProcessorEditor::handleSampleRateChange(double sampleRate, int bufferSize)
{
    if (_stnSpectrum != nullptr)
        _stnSpectrum->reset(bufferSize, sampleRate);
}

void
BLSTNAudioProcessorEditor::timerCallback()
{
    vector<float> noiseBuffer;
    vector<float> sinesBuffer;
    vector<float> sumBuffer;
        
    bool newBuffersAvailable = _audioProcessor.getBuffers(&noiseBuffer,
                                                          &sinesBuffer,
                                                          &sumBuffer);

    if (newBuffersAvailable)
    {
        _stnSpectrum->updateCurves(noiseBuffer,
                                   sinesBuffer,
                                   sumBuffer);
    }
        
#ifdef __linux__
    _spectrumComponent->repaint();
#endif
#ifdef __APPLE__
    _spectrumComponent->repaint();
#endif
}
