#pragma once

#include <JuceHeader.h>

class BitmapCheckBox : public juce::Component
{
public:
    BitmapCheckBox()
    {
        // Load the images from BinaryData
        uncheckedBitmap = juce::ImageFileFormat::loadFrom(BinaryData::checkbox_unchecked_png, BinaryData::checkbox_unchecked_pngSize);
        checkedBitmap = juce::ImageFileFormat::loadFrom(BinaryData::checkbox_checked_png, BinaryData::checkbox_checked_pngSize);

        // Ensure the images loaded successfully
        jassert(uncheckedBitmap.isValid());
        jassert(checkedBitmap.isValid());

        // Ensure the component is set to be clickable
        setInterceptsMouseClicks(true, true);
    }

    void paint(juce::Graphics& g) override
    {
        // Draw the appropriate bitmap depending on the state
        if (isChecked)
            g.drawImage(checkedBitmap, getLocalBounds().toFloat());
        else
            g.drawImage(uncheckedBitmap, getLocalBounds().toFloat());
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        // Toggle the state when the user clicks
        isChecked = !isChecked;
        repaint();

        // Notify listeners of the change
        if (onStateChange)
            onStateChange(isChecked);
    }

    // Set whether the checkbox is checked or unchecked
    void setChecked(bool shouldBeChecked)
    {
        if (isChecked != shouldBeChecked)
        {
            isChecked = shouldBeChecked;
            repaint();
        }
    }

    // Get the current state of the checkbox
    bool getChecked() const { return isChecked; }

    // Set a callback to be triggered when the state changes
    std::function<void(bool)> onStateChange;

private:
    juce::Image uncheckedBitmap;
    juce::Image checkedBitmap;
    bool isChecked = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitmapCheckBox)
};
