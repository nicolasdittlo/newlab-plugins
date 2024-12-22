#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_core/juce_core.h>

class FontManager
{
public:
    // Get the singleton instance
    static FontManager& getInstance()
    {
        static FontManager _instance;
        static bool _initialized = false;
        if (!_initialized)
        {
            _instance.initializeFonts();
            _initialized = true;
        }
        return _instance;
    }

    // Deleted to prevent copying or assignment
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    // Retrieve a Font object by name
    juce::Font getFont(const juce::String& fontName, float fontSize = 12.0f) const
    {
        auto typeface = _fonts[fontName];
        if (typeface != nullptr)
        {
            return juce::Font(juce::FontOptions(typeface)).withHeight(fontSize);
        }
        return juce::Font(juce::FontOptions()); // Return default font if not found
    }

private:
    // Private constructor for Singleton
    FontManager() = default;

    // Load predefined fonts from binary data
    void initializeFonts()
    {
        addFontFromBinary(BinaryData::fontbold_ttf, BinaryData::fontbold_ttfSize, "Font-Bold");
        addFontFromBinary(BinaryData::OpenSansExtraBold_ttf, BinaryData::OpenSansExtraBold_ttfSize, "OpenSans-ExtraBold");
        addFontFromBinary(BinaryData::RobotoBold_ttf, BinaryData::RobotoBold_ttfSize, "Roboto-Bold");
        addFontFromBinary(BinaryData::fontregular_ttf, BinaryData::fontregular_ttfSize, "Font-Regular");
    }

    // Add a font from binary data
    bool addFontFromBinary(const void* fontData, size_t dataSize, const juce::String& fontName)
    {
        auto typeface = juce::Typeface::createSystemTypefaceFor(fontData, static_cast<int>(dataSize));
        if (typeface == nullptr)
            return false;

        _fonts.set(fontName, typeface);
        return true;
    }

    // Store fonts in a map
    juce::HashMap<juce::String, juce::Typeface::Ptr> _fonts;
};
