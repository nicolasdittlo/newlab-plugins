/* Copyright (C) 2025 Nicolas Dittlo <newlab.plugins@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this software; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

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
        attributedString.setFont(FontManager::getInstance()->getFont("Roboto-Bold", 13.0));

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
        auto y = height - textHeight - 7;

        // Draw the text layout
        textLayout.draw(g, juce::Rectangle<float>(x, y, textWidth, textHeight));
    }
};
