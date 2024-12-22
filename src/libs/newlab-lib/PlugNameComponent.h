#pragma once

#include <JuceHeader.h>

class PlugNameComponent : public juce::Component
{
public:
    PlugNameComponent()
    {
        // Load the image from the binary resources
        auto imageFile = juce::ImageFileFormat::loadFrom(BinaryData::plugname_png, BinaryData::plugname_pngSize);

        if (imageFile.isValid())
        {
            _image = imageFile;
            // Set the size of the component to match the native size of the image
            setSize(_image.getWidth(), _image.getHeight());
        }
        else
        {
            // Handle the case where the image failed to load
            jassertfalse; // This will trigger a breakpoint in debug mode
            setSize(100, 100); // Fallback size
        }
    }

    void paint(juce::Graphics& g) override
    {
        if (_image.isValid())
        {
            // Draw the image
            g.drawImage(_image, getLocalBounds().toFloat(), juce::RectanglePlacement::centred);
        }
    }

private:
    juce::Image _image;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlugNameComponent)
};
