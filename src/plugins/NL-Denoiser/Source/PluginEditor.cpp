#include "PluginProcessor.h"
#include "PluginEditor.h"

NLDenoiserAudioProcessorEditor::NLDenoiserAudioProcessorEditor(NLDenoiserAudioProcessor& p)
    : AudioProcessorEditor(&p), _audioProcessor(p)
{
    // Load the background image from binary resources
    _backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);

    // Configure the ratio slider with units
    _ratioSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::BigSlider);
    _ratioSlider->setRange(0.0, 100.0, 0.1);
    _ratioSlider->setDefaultValue(100.0);

    // Add the rotary slider to the editor
    addAndMakeVisible(*_ratioSlider);

    // Configure the threshold slider with units
    _thresholdSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider, 0.25);
    _thresholdSlider->setRange(0.0, 100.0, 0.01);
    _thresholdSlider->setDefaultValue(0.1);

    // Add the rotary slider to the editor
    addAndMakeVisible(*_thresholdSlider);

    // Configure the transient boost slider with units
    _transBoostSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider);
    _transBoostSlider->setRange(0.0, 100.0, 0.1);
    _transBoostSlider->setDefaultValue(0.0);

    // Add the rotary slider to the editor
    addAndMakeVisible(*_transBoostSlider);

    // Configure the residual noise threshold slider with units
    _resNoiseThrsSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider);
    _resNoiseThrsSlider->setRange(0.0, 100.0, 0.1);
    _resNoiseThrsSlider->setDefaultValue(0.0);

    // Add the rotary slider to the editor
    addAndMakeVisible(*_resNoiseThrsSlider);

    // Add the learn check box to the editor
    addAndMakeVisible(_learnCheckBox);

    // Add the noise only check box to the editor
    addAndMakeVisible(_noiseOnlyCheckBox);

    // Add the soft denoise check box to the editor
    addAndMakeVisible(_autoResNoiseCheckBox);

    _qualityComboBox = std::make_unique<CustomComboBox>();
    _qualityComboBox->addItem("1 - Fast", 1);
    _qualityComboBox->addItem("2", 2);
    _qualityComboBox->addItem("3", 3);
    _qualityComboBox->addItem("4 - Best", 4);

    addAndMakeVisible(*_qualityComboBox);
    
    // Set the editor's size
    setSize(464, 464);
}

NLDenoiserAudioProcessorEditor::~NLDenoiserAudioProcessorEditor() = default;

void NLDenoiserAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Clear the background with a default color
    g.fillAll(juce::Colours::black);

    // Draw the background image
    if (_backgroundImage.isValid())
    {
        g.drawImageAt(_backgroundImage, 0, 0);
    }
}

void NLDenoiserAudioProcessorEditor::resized()
{
    auto bigSliderWidth = 72;
    auto bigSliderHeight = 72 + 25 + 20; // 72 for slider, 25 for spacing, 20 for label height
    _ratioSlider->setBounds(172, 282, bigSliderWidth, bigSliderHeight);

    auto smallSliderWidth = 72; // Updated width to match the label width for small sliders
    auto smallSliderHeight = 36 + 25 + 20; // 36 for slider, 25 for spacing, 20 for label height
    _thresholdSlider->setBounds(281 - (smallSliderWidth - 36) / 2, // Center the slider
                                316,
                                smallSliderWidth,
                                smallSliderHeight);

    _transBoostSlider->setBounds(281 - (smallSliderWidth - 36) / 2, // Center the slider
                                 218,
                                 smallSliderWidth,
                                 smallSliderHeight);

    _resNoiseThrsSlider->setBounds(372 - (smallSliderWidth - 36) / 2, // Center the slider
                                   316,
                                   smallSliderWidth,
                                   smallSliderHeight);

    _learnCheckBox.setBounds(32, 234, 20, 20);

    _noiseOnlyCheckBox.setBounds(32, 283, 20, 20);

    _autoResNoiseCheckBox.setBounds(32, 332, 20, 20);

    _qualityComboBox->setBounds(348, 255, 90, 20);
}
