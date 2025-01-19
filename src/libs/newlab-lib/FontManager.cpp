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

#include <JuceHeader.h>

#include "FontManager.h"

FontManager *FontManager::_instance = nullptr;

FontManager* FontManager::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new FontManager();
        _instance->initializeFonts();
    }
    return _instance;
}

juce::Font
FontManager::getFont(const juce::String& fontName, float fontSize) const
{
    auto typeface = _fonts[fontName];
    if (typeface != nullptr)
    {
        return juce::Font(juce::FontOptions(typeface)).withHeight(fontSize);
    }
    return juce::Font(juce::FontOptions()); // Return default font if not found
}
 
void
FontManager::initializeFonts()
{
    addFontFromBinary(BinaryData::fontbold_ttf, BinaryData::fontbold_ttfSize, "Font-Bold");
    addFontFromBinary(BinaryData::OpenSansExtraBold_ttf, BinaryData::OpenSansExtraBold_ttfSize, "OpenSans-ExtraBold");
    addFontFromBinary(BinaryData::RobotoBold_ttf, BinaryData::RobotoBold_ttfSize, "Roboto-Bold");
    addFontFromBinary(BinaryData::fontregular_ttf, BinaryData::fontregular_ttfSize, "Font-Regular");
}

bool
FontManager::addFontFromBinary(const void* fontData, size_t dataSize,
                               const juce::String& fontName)
{
    auto typeface = juce::Typeface::createSystemTypefaceFor(fontData, static_cast<int>(dataSize));
    if (typeface == nullptr)
        return false;
    
    _fonts.set(fontName, typeface);
    return true;
}
