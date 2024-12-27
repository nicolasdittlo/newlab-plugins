#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Config.h"
#include "CustomLookAndFeel.h"
#include "VersionTextDrawer.h"
#include "ManualPdfViewer.h"
#include "DemoTextDrawer.h"

#define VERSION_STR "7.0.0"

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
    _ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "ratio", _ratioSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_ratioSlider);

    // Configure the threshold slider with units
    _thresholdSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider, 0.25);
    _thresholdSlider->setRange(0.0, 100.0, 0.01);
    _thresholdSlider->setDefaultValue(0.1);
    _thresholdSlider->setTooltip("Threshold - Noise suppression threshold");
    _thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "threshold", _thresholdSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_thresholdSlider);

    // Configure the transient boost slider with units
    _transBoostSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider);
    _transBoostSlider->setRange(0.0, 100.0, 0.1);
    _transBoostSlider->setDefaultValue(0.0);
    _transBoostSlider->setTooltip("Transient Boost - Boost output transients");
    _transBoostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "transientBoost", _transBoostSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_transBoostSlider);

    // Configure the residual noise threshold slider with units
    _resNoiseThrsSlider = std::make_unique<RotarySliderWithValue>("", "%", SliderSize::SmallSlider);
    _resNoiseThrsSlider->setRange(0.0, 100.0, 0.1);
    _resNoiseThrsSlider->setDefaultValue(0.0);
    _resNoiseThrsSlider->setTooltip("Residual Noise - Residual denoise threshold");
    _resNoiseThrsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (_audioProcessor._parameters, "residualNoise", _resNoiseThrsSlider->getSlider());
    
    // Add the rotary slider to the editor
    addAndMakeVisible(*_resNoiseThrsSlider);

    // learn check box
    _learnCheckBox.setTooltip("Learn Mode - Learn the noise profile");

    _learnCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
        (_audioProcessor._parameters, "learnModeParamID", _learnCheckBox);

    // Add the learn check box to the editor
    addAndMakeVisible(_learnCheckBox);
    
    // noise only check box
    _noiseOnlyCheckBox.setTooltip("Noise Only - Output the suppressed noise instead of the signal");

     _noiseOnlyCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
         (_audioProcessor._parameters, "noiseOnlyParamID", _noiseOnlyCheckBox);
     
     // Add the noise only check box to the editor
    addAndMakeVisible(_noiseOnlyCheckBox);

    // soft denoise checkbox
    _autoResNoiseCheckBox.setTooltip("Soft Denoise - Automatically remove residual noise");

    _autoResNoiseCheckBoxAttachment = std::make_unique<BitmapCheckBoxAttachment>
        (_audioProcessor._parameters, "softDenoiseParamID", _autoResNoiseCheckBox);
    
    // Add the soft denoise check box to the editor
    addAndMakeVisible(_autoResNoiseCheckBox);

    // quality combo box
    _qualityComboBox = std::make_unique<CustomComboBox>();
    _qualityComboBox->addItem("1 - Fast", 1);
    _qualityComboBox->addItem("2", 2);
    _qualityComboBox->addItem("3", 3);
    _qualityComboBox->addItem("4 - Best", 4);

    _qualityComboBox->setTooltip("Quality - Processing quality");

    _qualityComboBoxAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        (_audioProcessor._parameters, "quality", *_qualityComboBox);
    
    addAndMakeVisible(*_qualityComboBox);

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

    _spectrumComponent = std::make_unique<SpectrumComponent>();
    addAndMakeVisible(*_spectrumComponent);

    _spectrumView = std::make_unique<SpectrumView>();
    _denoiserSpectrum = std::make_unique<DenoiserSpectrum>(_spectrumView.get(), 44100.0, 2048);

    _spectrumComponent->setSpectrumView(_spectrumView.get());
    
    // Set the editor's size
    setSize(464, 464);

    // Register the sample rate change listener
    _audioProcessor.setSampleRateChangeListener([this](double sampleRate, int bufferSize)
    {
        juce::MessageManager::callAsync([this, sampleRate, bufferSize]()
        {
            handleSampleRateChange(sampleRate, bufferSize);
        });
    });
    
    startTimer(15);
}

NLDenoiserAudioProcessorEditor::~NLDenoiserAudioProcessorEditor()
{
    stopTimer();
    
    // Reset the LookAndFeel to avoid dangling references
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void NLDenoiserAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Clear the background with a default color
    g.fillAll(juce::Colours::black);

    // Draw the background image
    if (_backgroundImage.isValid())
    {
        g.drawImageAt(_backgroundImage, 0, 0);
    }

    // Call the function to draw the version text
    VersionTextDrawer::drawVersionText(*this, g, VERSION_STR);

    // Please be kind, do not modify this code and share binaries
#if DEMO_VERSION
    DemoTextDrawer::drawDemoText(*this, g, "DEMO");
#endif

    // Grey out the res noise threshold slider if we use auto residual denoise
    auto* autoResNoiseValue = _audioProcessor._parameters.getRawParameterValue("softDenoiseParamID");
    if (autoResNoiseValue != nullptr)
    {
        float paramValue = autoResNoiseValue->load(); // Read the parameter value

        _resNoiseThrsSlider->setEnabled(paramValue < 0.5);
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

    _plugNameComponent->setBounds(getWidth()/2 - _plugNameComponent->getWidth()/2,
                                  getHeight() - _plugNameComponent->getHeight() - 15.0,
                                  _plugNameComponent->getWidth(),
                                  _plugNameComponent->getHeight());

    _helpButton->setBounds(getWidth() - 20 - 14, getHeight() - 20 - 10, 20, 20);

    _spectrumComponent->setBounds(0, 0, 464, 198);
}

void NLDenoiserAudioProcessorEditor::handleSampleRateChange(double sampleRate, int bufferSize)
{
    _denoiserSpectrum->reset(bufferSize, sampleRate);
}

void
NLDenoiserAudioProcessorEditor::timerCallback()
{
    vector<float> signalBuffer;
    vector<float> noiseBuffer;
    vector<float> noiseProfileBuffer;
        
    bool newBuffersAvailable = _audioProcessor.getBuffers(&signalBuffer,
                                                          &noiseBuffer,
                                                          &noiseProfileBuffer);

    if (newBuffersAvailable)
    {
        bool isLearning = _audioProcessor._parameters.getRawParameterValue("learnModeParamID")->load();

        if (!isLearning)
        {
            float threshold = _audioProcessor._parameters.getRawParameterValue("threshold")->load();
            threshold *= 0.01;
            
            DenoiserProcessor::applyThresholdValueToNoiseCurve(&noiseProfileBuffer, threshold);
        }
        
        _denoiserSpectrum->updateCurves(signalBuffer,
                                        noiseBuffer,
                                        noiseProfileBuffer,
                                        isLearning);
    }

    _spectrumComponent->repaint();
}
