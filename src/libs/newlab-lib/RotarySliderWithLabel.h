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
            // Calculate rotation
            auto rotation = juce::jmap(getValue(), getMinimum(), getMaximum(), 0.0, juce::MathConstants<double>::twoPi);

            // Draw rotated SVG
            auto bounds = getLocalBounds().toFloat().reduced(10.0f);
            auto center = bounds.getCentre();

            g.addTransform(juce::AffineTransform::rotation(rotation, center.x, center.y));
            knobDrawable->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }

private:
    std::unique_ptr<juce::Drawable> knobDrawable;
};

class RotarySliderWithLabel : public juce::Component
{
public:
    RotarySliderWithLabel(const juce::String& sliderTitle, const juce::String& units)
        : titleLabel("TitleLabel", sliderTitle),
          valueLabel("ValueLabel", "")
    {
        // Slider setup
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.onValueChange = [this]() { updateValueLabel(); };
        addAndMakeVisible(slider);

        // Title label setup
        titleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(titleLabel);

        // Value label setup
        valueLabel.setEditable(true);
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.onTextChange = [this]() { updateSliderFromLabel(); };
        addAndMakeVisible(valueLabel);

        this->units = units;
        updateValueLabel();
    }

    void resized() override
    {
        auto area = getLocalBounds();

        // Layout
        slider.setBounds(area.removeFromTop(area.getHeight() - 40).reduced(10));
        titleLabel.setBounds(area.removeFromTop(20));
        valueLabel.setBounds(area);
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
    juce::Label titleLabel;
    juce::Label valueLabel;
    juce::String units;
    double defaultValue = 0.0; // Default value for the slider
};
