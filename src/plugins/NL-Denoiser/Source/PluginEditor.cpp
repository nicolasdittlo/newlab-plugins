#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NLDenoiserAudioProcessorEditor::NLDenoiserAudioProcessorEditor (NLDenoiserAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Load the background image from binary resources
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);

    // Configure the ratio slider with units
    ratioSlider = std::make_unique<RotarySliderWithValue>("", "%");
    ratioSlider->setRange(0.0, 100.0, 0.1);
    ratioSlider->setDefaultValue(100.0);

    // Add the rotary slider to the editor
    addAndMakeVisible(*ratioSlider);

    // Set the editor's size
    setSize (464, 464);
}

NLDenoiserAudioProcessorEditor::~NLDenoiserAudioProcessorEditor() = default;

//==============================================================================
void NLDenoiserAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Clear the background with a default color
    g.fillAll(juce::Colours::black);

    // Draw the background image
    if (backgroundImage.isValid())
    {
        g.drawImageAt(backgroundImage, 0, 0);
    }
}

void NLDenoiserAudioProcessorEditor::resized()
{
    // Center the rotary slider
    auto sliderWidth = 72;
    auto sliderHeight = 72 + 30 + 20; // 72 for slider, 30 for spacing, 20 for label height
    ratioSlider->setBounds(172, 282,
                           sliderWidth,
                           sliderHeight);
}
