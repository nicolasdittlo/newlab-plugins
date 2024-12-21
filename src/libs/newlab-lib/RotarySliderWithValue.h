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
        : _sliderSize(size)
    {
        setSkewFactor(skewFactor); // Apply the skew factor

        // Load SVG from resources
        auto svgData = BinaryData::knob_svg;
        auto svgSize = BinaryData::knob_svgSize;
        auto svgDrawable = juce::Drawable::createFromImageData(svgData, svgSize);
        if (svgDrawable)
        {
            _knobDrawable = std::unique_ptr<juce::Drawable>(svgDrawable.release());
        }
    }

    void setRange(double min, double max, double interval)
    {
        juce::Slider::setRange(min, max, interval);
    }

    void setDefaultValue(double value)
    {
        _defaultValue = value;
    }

    double getDefaultValue() const
    {
        return _defaultValue;
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        if (isEnabled())
        {
            setValue(_defaultValue, juce::sendNotificationSync);
        }
    }

    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(juce::Colours::transparentBlack);

        if (_knobDrawable)
        {
            // Calculate rotation from -135 to 135 degrees
            auto rotation = juce::jmap(getValue(), getMinimum(), getMaximum(), -juce::MathConstants<double>::pi * 3.0 / 4.0, juce::MathConstants<double>::pi * 3.0 / 4.0);

            // Draw rotated SVG
            auto bounds = getLocalBounds().toFloat(); // Use full bounds for drawing
            auto center = bounds.getCentre();

            g.addTransform(juce::AffineTransform::rotation(rotation, center.x, center.y));
            _knobDrawable->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }

    void resized() override
    {
        int size = (_sliderSize == SliderSize::BigSlider) ? 72 : 36;
        setSize(size, size); // Ensure the size matches the specified size
    }

private:
    SliderSize _sliderSize;
    double _defaultValue = 0.0; // Default value for the slider
    std::unique_ptr<juce::Drawable> _knobDrawable;
};

class RotarySliderWithValue : public juce::Component
{
public:
    RotarySliderWithValue(const juce::String& sliderTitle, const juce::String& units, SliderSize size, double skewFactor = 1.0)
        : _valueLabel("ValueLabel", ""), _slider(size, skewFactor), _sliderSize(size)
    {
        // Slider setup
        _slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        _slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        _slider.onValueChange = [this]() { updateValueLabel(); };
        addAndMakeVisible(_slider);

        // Value label setup
        _valueLabel.setEditable(true);
        _valueLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        _valueLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#ff939393"));
        _valueLabel.setJustificationType(juce::Justification::centred);
        _valueLabel.onTextChange = [this]() { updateSliderFromLabel(); };
        addAndMakeVisible(_valueLabel);

        this->_units = units;
        updateValueLabel();
    }

    void resized() override
    {
        int sliderDimension = (_sliderSize == SliderSize::BigSlider) ? 72 : 36;
        // Define fixed bounds for the slider within this component
        auto sliderX = (getWidth() - sliderDimension) / 2; // Center the slider horizontally
        auto sliderY = 0; // Start at the top of the component
        _slider.setBounds(sliderX, sliderY, sliderDimension, sliderDimension);

        // Position the value label below the slider
        int labelWidth = (_sliderSize == SliderSize::BigSlider) ? sliderDimension : 72; // Allow label to exceed for small sliders
        auto valueLabelArea = juce::Rectangle<int>(sliderX - (labelWidth - sliderDimension) / 2, _slider.getBottom() + 25, labelWidth, 20);
        _valueLabel.setBounds(valueLabelArea);
    }

    void setRange(double min, double max, double interval)
    {
        _slider.setRange(min, max, interval);
    }

    void setDefaultValue(double value)
    {
        _slider.setDefaultValue(value); // Forward to CustomRotarySlider
        _defaultValue = value; // Keep for internal tracking
    }

    void setValue(double value)
    {
        _slider.setValue(value);
    }

    double getValue() const
    {
        return _slider.getValue();
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        _slider.mouseDoubleClick(event);
    }

private:
    void updateValueLabel()
    {
        _valueLabel.setText(juce::String(_slider.getValue(), 2) + " " + _units, juce::dontSendNotification);
    }

    void updateSliderFromLabel()
    {
        auto text = _valueLabel.getText().trim();
        auto value = text.upToFirstOccurrenceOf(" ", false, false).getDoubleValue();
        _slider.setValue(value);
    }

    CustomRotarySlider _slider;
    juce::Label _valueLabel;
    SliderSize _sliderSize;
    juce::String _units;
    double _defaultValue = 0.0; // Default value for the slider
};
