#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CustomLookAndFeel.h"

NLDenoiserAudioProcessorEditor::NLDenoiserAudioProcessorEditor(NLDenoiserAudioProcessor& p)
    : AudioProcessorEditor(&p), _audioProcessor(p)
{
    // Set the custom look and feel
    juce::LookAndFeel::setDefaultLookAndFeel(new CustomLookAndFeel());
 
    // Load the background image from binary resources
    _backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);

    // Configure the ratio slider with units
    _ratioSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::BigSlider);
    _ratioSlider->setRange(0.0, 100.0, 0.1);
    _ratioSlider->setDefaultValue(100.0);
    _ratioSlider->setTooltip("Ratio - Noise suppression ratio");
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_ratioSlider);

    // Configure the threshold slider with units
    _thresholdSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider, 0.25);
    _thresholdSlider->setRange(0.0, 100.0, 0.01);
    _thresholdSlider->setDefaultValue(0.1);
    _thresholdSlider->setTooltip("Threshold - Noise suppression threshold");
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_thresholdSlider);

    // Configure the transient boost slider with units
    _transBoostSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider);
    _transBoostSlider->setRange(0.0, 100.0, 0.1);
    _transBoostSlider->setDefaultValue(0.0);
    _transBoostSlider->setTooltip("Transient Boost - Boost output transients");
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_transBoostSlider);

    // Configure the residual noise threshold slider with units
    _resNoiseThrsSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider);
    _resNoiseThrsSlider->setRange(0.0, 100.0, 0.1);
    _resNoiseThrsSlider->setDefaultValue(0.0);
    _resNoiseThrsSlider->setTooltip("Residual Noise - Residual denoise threshold");
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_resNoiseThrsSlider);

    // learn check box
    _learnCheckBox.setTooltip("Learn Mode - Learn the noise profile");
    
    // Add the learn check box to the editor
    addAndMakeVisible(_learnCheckBox);

    // noise only check box
    _noiseOnlyCheckBox.setTooltip("Noise Only - Output the suppressed noise instead of the signal");
    
    // Add the noise only check box to the editor
    addAndMakeVisible(_noiseOnlyCheckBox);

    // soft denoise checkbox
    _autoResNoiseCheckBox.setTooltip("Soft Denoise - Automatically remove residual noise");
        
    // Add the soft denoise check box to the editor
    addAndMakeVisible(_autoResNoiseCheckBox);

    // quality combo box
    _qualityComboBox = std::make_unique<CustomComboBox>();
    _qualityComboBox->addItem("1 - Fast", 1);
    _qualityComboBox->addItem("2", 2);
    _qualityComboBox->addItem("3", 3);
    _qualityComboBox->addItem("4 - Best", 4);

    _qualityComboBox->setTooltip("Quality - Processing quality");
        
    addAndMakeVisible(*_qualityComboBox);

    // Tooltip window
    _tooltipWindow = std::make_unique<juce::TooltipWindow>(this, 500);
    
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
