#pragma once

#include <JuceHeader.h>

enum class SliderSize
{
    SmallSlider,
    BigSlider
};

class CustomRotarySlider : public juce::Slider
{
public:
    CustomRotarySlider(SliderSize size, double skewFactor = 1.0)
        : sliderSize(size)
    {
        setSkewFactor(skewFactor); // Apply the skew factor

        // Load SVG from resources
        auto svgData = BinaryData::knob_svg;
        auto svgSize = BinaryData::knob_svgSize;
        auto svgDrawable = juce::Drawable::createFromImageData(svgData, svgSize);
        if (svgDrawable)
        {
            knobDrawable = std::unique_ptr<juce::Drawable>(svgDrawable.release());
        }
    }

    void setRange(double min, double max, double interval)
    {
        juce::Slider::setRange(min, max, interval);
    }

    void setDefaultValue(double value)
    {
        defaultValue = value;
    }

    double getDefaultValue() const
    {
        return defaultValue;
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        if (isEnabled())
        {
            setValue(defaultValue, juce::sendNotificationSync);
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
        int size = (sliderSize == SliderSize::BigSlider) ? 72 : 36;
        setSize(size, size); // Ensure the size matches the specified size
    }

private:
    SliderSize sliderSize;
    double defaultValue = 0.0; // Default value for the slider
    std::unique_ptr<juce::Drawable> knobDrawable;
};

class RotarySliderWithValue : public juce::Component
{
public:
    RotarySliderWithValue(const juce::String& sliderTitle, const juce::String& units, SliderSize size, double skewFactor = 1.0)
        : valueLabel("ValueLabel", ""), slider(size, skewFactor), sliderSize(size)
    {
        // Slider setup
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.onValueChange = [this]() { updateValueLabel(); };
        addAndMakeVisible(slider);

        // Value label setup
        valueLabel.setEditable(true);
        valueLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        valueLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#ff939393"));
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.onTextChange = [this]() { updateSliderFromLabel(); };
        addAndMakeVisible(valueLabel);

        this->units = units;
        updateValueLabel();
    }

    void resized() override
    {
        int sliderDimension = (sliderSize == SliderSize::BigSlider) ? 72 : 36;
        // Define fixed bounds for the slider within this component
        auto sliderX = (getWidth() - sliderDimension) / 2; // Center the slider horizontally
        auto sliderY = 0; // Start at the top of the component
        slider.setBounds(sliderX, sliderY, sliderDimension, sliderDimension);

        // Position the value label below the slider
        int labelWidth = (sliderSize == SliderSize::BigSlider) ? sliderDimension : 72; // Allow label to exceed for small sliders
        auto valueLabelArea = juce::Rectangle<int>(sliderX - (labelWidth - sliderDimension) / 2, slider.getBottom() + 25, labelWidth, 20);
        valueLabel.setBounds(valueLabelArea);
    }

    void setRange(double min, double max, double interval)
    {
        slider.setRange(min, max, interval);
    }

    void setDefaultValue(double value)
    {
        slider.setDefaultValue(value); // Forward to CustomRotarySlider
        defaultValue = value; // Keep for internal tracking
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
        slider.mouseDoubleClick(event);
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
    SliderSize sliderSize;
    juce::String units;
    double defaultValue = 0.0; // Default value for the slider
};
