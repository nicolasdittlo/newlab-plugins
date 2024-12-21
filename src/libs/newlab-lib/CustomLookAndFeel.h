#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override
    {
        g.fillAll(juce::Colour::fromString("#ff19193b"));
        g.setColour(juce::Colour::fromString("#ff565667"));
        g.drawRect(0, 0, width, height, 2.0);
        g.setColour(juce::Colour::fromString("#ffd1d8df"));
        g.setFont(juce::FontOptions(16.0f)); // Change font size
        g.drawText(text, 0, 0, width, height, juce::Justification::centred);
    }

    // Override to calculate and return custom tooltip bounds
    juce::Rectangle<int> getTooltipBounds(const juce::String& tipText,
                                          juce::Point<int> screenPos,
                                          juce::Rectangle<int> parentArea) override
    {
        // Define font for text layout
        juce::Font font(juce::FontOptions(16.0f)); // Ensure the font matches the one in drawTooltip

        // Calculate text layout size
        juce::AttributedString attributedString;
        attributedString.setFont(font);
        attributedString.setText(tipText);
        attributedString.setColour(juce::Colours::black);

        juce::TextLayout textLayout;
        textLayout.createLayout(attributedString, 300.0f); // Limit width if necessary

        // Calculate bounds from the overall text layout
        auto bounds = textLayout.getStringBounds(font, tipText);
        int w = static_cast<int>(bounds.getWidth() + 14.0f); // Add padding
        int h = static_cast<int>(bounds.getHeight() + 10.0f);

        // Add more padding
        w += 10;
        h += 10;
        
        // Calculate tooltip position
        int x = (screenPos.x > parentArea.getCentreX()) ? screenPos.x - (w + 12) : screenPos.x + 24;
        int y = (screenPos.y > parentArea.getCentreY()) ? screenPos.y - (h + 6) : screenPos.y + 6;

        // Ensure the tooltip is within the parent area
        return juce::Rectangle<int>(x, y, w, h).constrainedWithin(parentArea);
    }
};
