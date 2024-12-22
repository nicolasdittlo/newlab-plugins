#pragma once

#include <JuceHeader.h>

class BitmapCheckBox : public juce::Component, public juce::SettableTooltipClient
{
public:
    BitmapCheckBox()
    {
        // Load the images from BinaryData
        _uncheckedBitmap = juce::ImageFileFormat::loadFrom(BinaryData::checkbox_unchecked_png, BinaryData::checkbox_unchecked_pngSize);
        _checkedBitmap = juce::ImageFileFormat::loadFrom(BinaryData::checkbox_checked_png, BinaryData::checkbox_checked_pngSize);

        // Ensure the images loaded successfully
        jassert(_uncheckedBitmap.isValid());
        jassert(_checkedBitmap.isValid());

        // Ensure the component is set to be clickable
        setInterceptsMouseClicks(true, true);
    }

    void paint(juce::Graphics& g) override
    {
        // Draw the appropriate bitmap depending on the state
        if (_isChecked)
            g.drawImage(_checkedBitmap, getLocalBounds().toFloat());
        else
            g.drawImage(_uncheckedBitmap, getLocalBounds().toFloat());
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        // Toggle the state when the user clicks
        _isChecked = !_isChecked;
        repaint();

        // Notify listeners of the change
        if (onStateChange)
            onStateChange(_isChecked);
    }

    // Set whether the checkbox is checked or unchecked
    void setChecked(bool shouldBeChecked)
    {
        if (_isChecked != shouldBeChecked)
        {
            _isChecked = shouldBeChecked;
            repaint();
        }
    }

    // Get the current state of the checkbox
    bool getChecked() const { return _isChecked; }

    // Set a callback to be triggered when the state changes
    std::function<void(bool)> onStateChange;

private:
    juce::Image _uncheckedBitmap;
    juce::Image _checkedBitmap;
    bool _isChecked = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitmapCheckBox)
};

class BitmapCheckBoxAttachment : public juce::AudioProcessorValueTreeState::Listener
{
public:
    BitmapCheckBoxAttachment(juce::AudioProcessorValueTreeState& stateToUse, 
                             const juce::String& parameterID, 
                             BitmapCheckBox& checkBox)
        : _apvts(stateToUse), _paramID(parameterID), _checkBox(checkBox)
    {
        auto* parameter = _apvts.getParameter(_paramID);
        jassert(parameter != nullptr);

        // Add listener for parameter changes
        _apvts.addParameterListener(_paramID, this);

        // Initialize checkbox state based on parameter
        auto initialValue = parameter->getValue();
        _checkBox.setChecked(initialValue > 0.5f);

        // Register for checkbox state changes
        _checkBox.onStateChange = [this](bool isChecked) {
            if (auto* parameter = _apvts.getParameter(_paramID))
                parameter->setValueNotifyingHost(isChecked ? 1.0f : 0.0f);
        };
    }

    ~BitmapCheckBoxAttachment()
    {
        _apvts.removeParameterListener(_paramID, this);
    }

    void parameterChanged(const juce::String& parameterID, float newValue) override
    {
        if (parameterID == _paramID)
        {
            _checkBox.setChecked(newValue > 0.5f);
        }
    }

private:
    juce::AudioProcessorValueTreeState& _apvts;
    juce::String _paramID;
    BitmapCheckBox& _checkBox;
};
