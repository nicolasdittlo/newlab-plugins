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

#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include <juce_core/juce_core.h>

class FontManager
{
public:
    // Get the singleton instance
    static FontManager* getInstance();

    // Deleted to prevent copying or assignment
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    // Retrieve a Font object by name
    juce::Font getFont(const juce::String& fontName, float fontSize = 12.0f) const;

private:
    // Private constructor for Singleton
    FontManager() = default;

    // Load predefined fonts from binary data
    void initializeFonts();

    // Add a font from binary data
    bool addFontFromBinary(const void* fontData, size_t dataSize, const juce::String& fontName);

    // Store fonts in a map
    juce::HashMap<juce::String, juce::Typeface::Ptr> _fonts;
    
    static FontManager *_instance;
};

#endif
