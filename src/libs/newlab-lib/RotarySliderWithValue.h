#pragma once

#include <JuceHeader.h>

class CustomRotarySlider : public juce::Slider
{
public:
    CustomRotarySlider()
    {
        // Load SVG from resources
        auto svgData = BinaryData::knob_svg;
        auto svgSize = BinaryData::knob_svgSize;
        auto svgDrawable = juce::Drawable::createFromImageData(svgData, svgSize);
        if (svgDrawable)
        {
            knobDrawable = std::unique_ptr<juce::Drawable>(svgDrawable.release());
        }
    }

    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(juce::Colours::transparentBlack);

        if (knobDrawable)
        {
            // Calculate rotation from -135 to 135 degrees
            auto rotation = juce::jmap(getValue(), getMinimum(), getMaximum(), -juce::MathConstants<double>::pi * 3.0 / 4.0, juce::MathConstants<double>::pi * 3.0 / 4.0);

            // Draw rotated SVG
            auto bounds = getLocalBounds().toFloat(); // Use full bounds for drawing
            auto center = bounds.getCentre();

            g.addTransform(juce::AffineTransform::rotation(rotation, center.x, center.y));
            knobDrawable->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }

    void resized() override
    {
        setSize(72, 72); // Ensure the size is fixed to 72x72 pixels
    }

private:
    std::unique_ptr<juce::Drawable> knobDrawable;
};

class RotarySliderWithValue : public juce::Component
{
public:
    RotarySliderWithValue(const juce::String& sliderTitle, const juce::String& units)
        : valueLabel("ValueLabel", "")
    {
        // Slider setup
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.onValueChange = [this]() { updateValueLabel(); };
        addAndMakeVisible(slider);

        // Value label setup
        valueLabel.setEditable(true);
        valueLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        valueLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#ff939393"));
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.onTextChange = [this]() { updateSliderFromLabel(); };
        addAndMakeVisible(valueLabel);

        this->units = units;
        updateValueLabel();
    }

    void resized() override
    {
        // Define fixed bounds for the slider within this component
        auto sliderX = (getWidth() - 72) / 2; // Center the slider horizontally
        auto sliderY = 0; // Start at the top of the component
        slider.setBounds(sliderX, sliderY, 72, 72);

        // Position the value label 30 pixels below the bottom of the slider
        auto valueLabelArea = juce::Rectangle<int>(sliderX, slider.getBottom() + 25, 72, 20);
        valueLabel.setBounds(valueLabelArea);
    }

    void setRange(double min, double max, double interval)
    {
        slider.setRange(min, max, interval);
    }

    void setDefaultValue(double value)
    {
        defaultValue = value;
        slider.setValue(value); // Set the initial value to default
    }

    void setValue(double value)
    {
        slider.setValue(value);
    }

    double getValue() const
    {
        return slider.getValue();
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        if (event.eventComponent == &slider)
        {
            slider.setValue(defaultValue);
        }
    }

private:
    void updateValueLabel()
    {
        valueLabel.setText(juce::String(slider.getValue(), 2) + " " + units, juce::dontSendNotification);
    }

    void updateSliderFromLabel()
    {
        auto text = valueLabel.getText().trim();
        auto value = text.upToFirstOccurrenceOf(" ", false, false).getDoubleValue();
        slider.setValue(value);
    }

    CustomRotarySlider slider;
    juce::Label valueLabel;
    juce::String units;
    double defaultValue = 0.0; // Default value for the slider
};
