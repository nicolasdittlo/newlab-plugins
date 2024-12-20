/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NLDenoiserAudioProcessorEditor::NLDenoiserAudioProcessorEditor (NLDenoiserAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Load the background image from binary resources
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    
    // Configure the ratio slider with title and units
    ratioSlider = std::make_unique<RotarySliderWithLabel>("RATIO", "%");
    ratioSlider->setRange(0.0, 100.0, 0.1);
    ratioSlider->setDefaultValue(100.0);
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*ratioSlider);

        
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (464, 464);
}

NLDenoiserAudioProcessorEditor::~NLDenoiserAudioProcessorEditor()
{
}

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
    ratioSlider->setBounds(getLocalBounds().reduced(20));
}
