#pragma once

#include <JuceHeader.h>

class HelpButton : public juce::Component, public juce::SettableTooltipClient
{
public:
    HelpButton()
    {
        // Load the images from BinaryData
        _bitmap = juce::ImageFileFormat::loadFrom(BinaryData::help_button_png, BinaryData::help_button_pngSize);
        _bitmapOver = juce::ImageFileFormat::loadFrom(BinaryData::help_button_over_png, BinaryData::help_button_over_pngSize);

        // Ensure the images loaded successfully
        jassert(_bitmap.isValid());
        jassert(_bitmapOver.isValid());

        // Ensure the component is set to be clickable
        setInterceptsMouseClicks(true, true);
    }

    void paint(juce::Graphics& g) override
    {
        // Draw the appropriate bitmap depending on the state
        if (_isMouseOver)
            g.drawImage(_bitmapOver, getLocalBounds().toFloat());
        else
            g.drawImage(_bitmap, getLocalBounds().toFloat());
    }

    void mouseEnter(const juce::MouseEvent& event) override
    {
        _isMouseOver = true;
        repaint(); // Trigger a redraw
    }

    void mouseExit(const juce::MouseEvent& event) override
    {
        _isMouseOver = false;
        repaint(); // Trigger a redraw
    }
    
    void mouseUp(const juce::MouseEvent& event) override
    {
        // Notify listeners of the change
        if (onStateChange)
            onStateChange();
    }

    // Set a callback to be triggered when the state changes
    std::function<void(void)> onStateChange;

private:
    juce::Image _bitmap;
    juce::Image _bitmapOver;
    bool _isMouseOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelpButton)
};
