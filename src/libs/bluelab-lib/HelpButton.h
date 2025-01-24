/* Copyright (C) 2025 Nicolas Dittlo <bluelab.plugins@gmail.com>
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
