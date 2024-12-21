#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override
    {
        g.fillAll(juce::Colour::fromString("#ff19193b"));
        g.setColour(juce::Colour::fromString("#ff565667"));
        g.drawRect(0, 0, width, height);
        g.setColour(juce::Colour::fromString("#ffd1d8df"));
        g.setFont(juce::FontOptions(14.0f)); // Change font size
        g.drawText(text, 0, 0, width, height, juce::Justification::centred);
    }
};
