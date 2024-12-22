#pragma once

#include <JuceHeader.h>

class VersionTextDrawer
{
 public:
    static void drawVersionText(const juce::AudioProcessorEditor &editor, juce::Graphics& g, const juce::String &text)
    {
        // Create a TextLayout for better text handling
        juce::TextLayout textLayout;
        juce::AttributedString attributedString;

        // Set the text attributes
        attributedString.setText(text);
        attributedString.setColour(juce::Colour(0xff939393));
        attributedString.setFont(FontManager::getInstance().getFont("Roboto-Bold", 13.0));

        // Create the layout
        textLayout.createLayout(attributedString, editor.getWidth());

        // Get the dimensions of the editor
        auto width = editor.getWidth();
        auto height = editor.getHeight();

        // Get the text layout dimensions
        auto textWidth = textLayout.getWidth();
        auto textHeight = textLayout.getHeight();

        // Calculate the position for bottom-right alignment
        auto x = width - textWidth - 48;
        auto y = height - textHeight - 8;

        // Draw the text layout
        textLayout.draw(g, juce::Rectangle<float>(x, y, textWidth, textHeight));
    }
};
