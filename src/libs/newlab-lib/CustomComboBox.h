#include <JuceHeader.h>

class CustomComboBox : public juce::ComboBox
{
public:
    CustomComboBox()
    {
        // Customize ComboBox properties
        setJustificationType(juce::Justification::centred);
        setColour(juce::ComboBox::textColourId, juce::Colour(0xff939393)); // Text color
        setColour(juce::ComboBox::arrowColourId, juce::Colours::white); // Arrow color
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::black); // Background color

        setLookAndFeel(&popupLookAndFeel);
    }

    void paint(juce::Graphics& g) override
    {
        // Draw a black background when the ComboBox is not opened
        auto bounds = getLocalBounds();
        g.setColour(juce::Colours::black);
        g.fillRect(bounds);

        // Draw the text
        g.setColour(findColour(juce::ComboBox::textColourId));
        g.setFont(juce::FontOptions(getHeight() * 0.5f, juce::Font::bold));
        g.drawText(getText(), bounds.reduced(5), juce::Justification::centredLeft, true);

        // Draw the arrow
        g.setColour(findColour(juce::ComboBox::arrowColourId));
        auto arrowZone = bounds.removeFromRight(getHeight()).reduced(5);
        juce::Path arrow;
        arrow.addTriangle(arrowZone.getCentreX() - 5, arrowZone.getCentreY() - 3,
                          arrowZone.getCentreX() + 5, arrowZone.getCentreY() - 3,
                          arrowZone.getCentreX(), arrowZone.getCentreY() + 4);
        g.fillPath(arrow);
    }

private:
    class PopupLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
        {
            g.setColour(juce::Colour(0xff181838));
            g.fillRect(0, 0, width, height);

            g.setColour(juce::Colour(0xffffffff));
            g.drawRect(0, 0, width, height);
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               const bool isSeparator, const bool isActive,
                               const bool isHighlighted, const bool isTicked,
                               const bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText,
                               const juce::Drawable* icon, const juce::Colour* textColour) override
        {
            if (isSeparator)
            {
                g.setColour(juce::Colours::grey);
                g.drawLine((float)area.getX(), area.getCentreY(), (float)area.getRight(), area.getCentreY(), 1.0f);
                return;
            }

            auto r = area.reduced(4, 0);

            if (isHighlighted)
            {
                g.setColour(juce::Colour(0xff6666f5));
                g.fillRect(r);
            }

            g.setColour(isHighlighted ? juce::Colours::black : juce::Colours::white);
            g.setFont(juce::FontOptions(16.0f));
            g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

            if (isTicked)
            {
                juce::Path tick;
                auto tickBounds = r.removeFromRight(r.getHeight()).reduced(4);
                tick.startNewSubPath(tickBounds.getX(), tickBounds.getCentreY());
                tick.lineTo(tickBounds.getCentreX(), tickBounds.getBottom());
                tick.lineTo(tickBounds.getRight(), tickBounds.getY());
                g.setColour(juce::Colours::white);
                g.strokePath(tick, juce::PathStrokeType(2.0f));
            }

            if (hasSubMenu)
            {
                auto arrowZone = r.removeFromRight(r.getHeight()).reduced(4);
                juce::Path subMenuArrow;
                subMenuArrow.addTriangle(arrowZone.getX(), arrowZone.getY(),
                                         arrowZone.getRight(), arrowZone.getCentreY(),
                                         arrowZone.getX(), arrowZone.getBottom());
                g.setColour(juce::Colours::white);
                g.fillPath(subMenuArrow);
            }
        }
    };

    PopupLookAndFeel popupLookAndFeel;
};
