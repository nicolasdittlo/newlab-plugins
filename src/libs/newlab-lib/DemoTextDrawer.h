#pragma once

#include <JuceHeader.h>

class DemoTextDrawer
{
 public:
    static void drawDemoText(const juce::AudioProcessorEditor &editor, juce::Graphics& g, const juce::String &text)
    {
        // Create a TextLayout for better text handling
        juce::TextLayout textLayout;
        juce::AttributedString attributedString;

        // Set the text attributes
        attributedString.setText(text);
        attributedString.setColour(juce::Colour(0xffff0000));
        attributedString.setFont(FontManager::getInstance().getFont("OpenSans-ExtraBold", 16.0));

        // Create the layout
        textLayout.createLayout(attributedString, editor.getWidth());

        // Get the dimensions of the editor
        auto width = editor.getWidth();
        auto height = editor.getHeight();

        // Get the text layout dimensions
        auto textWidth = textLayout.getWidth();
        auto textHeight = textLayout.getHeight();

        // Calculate the position for bottom-left alignment
        auto x = 74;
        auto y = height - textHeight - 10;

        // Draw the text layout
        textLayout.draw(g, juce::Rectangle<float>(x, y, textWidth, textHeight));
    }
};
