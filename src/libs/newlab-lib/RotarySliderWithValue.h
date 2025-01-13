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

#pragma once

#include <JuceHeader.h>

#include "FontManager.h"

enum class SliderSize
{
    SmallSlider,
    BigSlider
};

class CustomRotarySlider : public juce::Slider
{
public:
    CustomRotarySlider(SliderSize size)
        : _sliderSize(size)
    {
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

    void setParamShape(double paramShape)
    {
        _paramShape = paramShape;
    }

    double valueToProportionOfLength(double value) override
    {
        double normValue = (value - getMinimum()) / (getMaximum() - getMinimum());
        return applyParamShape(normValue);
    }

    double proportionOfLengthToValue(double proportion) override
    {
        double shapedProportion = reverseParamShape(proportion);
        return shapedProportion * (getMaximum() - getMinimum()) + getMinimum();
    }

    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(juce::Colours::transparentBlack);

        if (_knobDrawable)
        {
            auto value = getValue();
            auto minimum = getMinimum();
            auto maximum = getMaximum();

            double normValue = valueToProportionOfLength(value);
            value = minimum + normValue * (maximum - minimum);

            // Calculate rotation from -135 to 135 degrees
            auto rotation = juce::jmap(value, minimum, maximum, -130.0 * juce::MathConstants<double>::pi / 180.0, 130.0 * juce::MathConstants<double>::pi / 180.0);

            // Draw rotated SVG
            auto bounds = getLocalBounds().toFloat(); // Use full bounds for drawing
            auto center = bounds.getCentre();

            g.addTransform(juce::AffineTransform::rotation(rotation, center.x, center.y));
            _knobDrawable->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }

        if (!isEnabled())
        {
            g.fillAll(juce::Colour(0xbb000000));
        }
    }

    void resized() override
    {
        int size = (_sliderSize == SliderSize::BigSlider) ? 72 : 36;
        setSize(size, size); // Ensure the size matches the specified size
    }

private:
    double applyParamShape(double normValue) const
    {
        return pow(normValue, _paramShape);
    }

    double reverseParamShape(double proportion) const
    {
        return pow(proportion, 1.0 / _paramShape);
    }

    SliderSize _sliderSize;
    double _defaultValue = 0.0; // Default value for the slider
    double _paramShape = 1.0;   // Computed parameter shape factor
    std::unique_ptr<juce::Drawable> _knobDrawable;
};

class RotarySliderWithValue : public juce::Component, public juce::SettableTooltipClient
{
public:
    RotarySliderWithValue(const juce::String& sliderTitle, const juce::String& units, SliderSize size)
        : _valueLabel("ValueLabel", ""), _slider(size), _sliderSize(size)
    {
        // Slider setup
        _slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        _slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        _slider.onValueChange = [this]() { updateValueLabel(); };
        addAndMakeVisible(_slider);

        // Value label setup
        _valueLabel.setEditable(true);
        _valueLabel.setFont(FontManager::getInstance().getFont("OpenSans-ExtraBold", 19.0f));
        _valueLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#ff939393"));
        _valueLabel.setJustificationType(juce::Justification::centred);
        _valueLabel.onTextChange = [this]() { updateSliderFromLabel(); };
        addAndMakeVisible(_valueLabel);

        this->_units = units;
        updateValueLabel();
    }

    CustomRotarySlider& getSlider()
    {
        return _slider;
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
        _interval = interval;
        
        updateValueLabel();
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

    void setParamShape(double paramShape)
    {
        _slider.setParamShape(paramShape);
    }

    double getValue() const
    {
        return _slider.getValue();
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        _slider.mouseDoubleClick(event);
    }

    void setTooltip(const juce::String& newTooltip) override
    {
        SettableTooltipClient::setTooltip(newTooltip);
        _slider.setTooltip(newTooltip);
    }

private:
    void updateValueLabel()
    {
        int decimalPlaces = std::max(0, static_cast<int>(-std::log10(_interval)));
        double value = _slider.getValue();
        if (decimalPlaces == 0)
            value = round(value);
        _valueLabel.setText(juce::String(value, decimalPlaces) + " " + _units, juce::dontSendNotification);
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
    double _interval = 0.0; // Interval for formatting
};
