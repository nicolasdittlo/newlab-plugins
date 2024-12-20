#include "PluginProcessor.h"
#include "PluginEditor.h"

NLDenoiserAudioProcessorEditor::NLDenoiserAudioProcessorEditor(NLDenoiserAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Load the background image from binary resources
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);

    // Configure the ratio slider with units
    ratioSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::BigSlider);
    ratioSlider->setRange(0.0, 100.0, 0.1);
    ratioSlider->setDefaultValue(100.0);

    // Add the rotary slider to the editor
    addAndMakeVisible(*ratioSlider);

    // Configure the threshold slider with units
    thresholdSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider, 0.25);
    thresholdSlider->setRange(0.0, 100.0, 0.01);
    thresholdSlider->setDefaultValue(0.1);

    // Add the rotary slider to the editor
    addAndMakeVisible(*thresholdSlider);

    // Configure the transient boost slider with units
    transBoostSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider);
    transBoostSlider->setRange(0.0, 100.0, 0.1);
    transBoostSlider->setDefaultValue(0.0);

    // Add the rotary slider to the editor
    addAndMakeVisible(*transBoostSlider);

    // Configure the residual noise threshold slider with units
    resNoiseThrsSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider);
    resNoiseThrsSlider->setRange(0.0, 100.0, 0.1);
    resNoiseThrsSlider->setDefaultValue(0.0);

    // Add the rotary slider to the editor
    addAndMakeVisible(*resNoiseThrsSlider);
    
    // Set the editor's size
    setSize(464, 464);
}

NLDenoiserAudioProcessorEditor::~NLDenoiserAudioProcessorEditor() = default;

void NLDenoiserAudioProcessorEditor::paint(juce::Graphics& g)
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
    auto bigSliderWidth = 72;
    auto bigSliderHeight = 72 + 25 + 20; // 72 for slider, 25 for spacing, 20 for label height
    ratioSlider->setBounds(172, 282, bigSliderWidth, bigSliderHeight);

    auto smallSliderWidth = 72; // Updated width to match the label width for small sliders
    auto smallSliderHeight = 36 + 25 + 20; // 36 for slider, 25 for spacing, 20 for label height
    thresholdSlider->setBounds(281 - (smallSliderWidth - 36) / 2, // Center the slider
                               316,
                               smallSliderWidth,
                               smallSliderHeight);

    transBoostSlider->setBounds(281 - (smallSliderWidth - 36) / 2, // Center the slider
                                218,
                                smallSliderWidth,
                                smallSliderHeight);

    resNoiseThrsSlider->setBounds(372 - (smallSliderWidth - 36) / 2, // Center the slider
                                  316,
                                  smallSliderWidth,
                                  smallSliderHeight);
}
