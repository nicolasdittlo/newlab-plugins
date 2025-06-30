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
