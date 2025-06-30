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

#include <string.h>

#include <JuceHeader.h>

#include "Axis.h"
#include "Curve.h"
#include "SpectrumViewJuce.h"

#define FONT "Roboto-Bold"
#define FONT_SIZE 12.0

SpectrumViewJuce::SpectrumViewJuce()
: _font(juce::FontOptions())
{
    _width = 256;
    _height = 256;
    
    auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::RobotoBold_ttf, BinaryData::RobotoBold_ttfSize);
    _font = juce::Font (juce::FontOptions(typeface));
}

SpectrumViewJuce::~SpectrumViewJuce() {}

void
SpectrumViewJuce::paint(juce::Graphics &g)
{
    drawAxis(g, true);

    g.saveState();
    drawCurves(g);
    g.restoreState();

    drawAxis(g, false);
    
    drawCurveDescriptions(g);

    drawSeparatorY0(g);
}

void
SpectrumViewJuce::drawAxis(juce::Graphics &g, bool lineLabelFlag)
{
    if (_hAxis != NULL)
        drawAxis(g, _hAxis, true, lineLabelFlag);
    
    if (_vAxis != NULL)
        drawAxis(g, _vAxis, false, lineLabelFlag);
}

void
SpectrumViewJuce::drawAxis(juce::Graphics &g, Axis *axis, bool horizontal, bool lineLabelFlag)
{
    g.saveState();
    
    g.setColour(juce::Colour((juce::uint8)axis->_color[0], (juce::uint8)axis->_color[1],
                             (juce::uint8)axis->_color[2], (juce::uint8)axis->_color[3]));
    
    for (int i = 0; i < axis->_values.size(); i++)
    {
        const Axis::Data &data = axis->_values[i];
        
        float t = data._t;
        const char *text = data._text.c_str();
        
        if (horizontal)
        {
            float textOffset = FONT_SIZE*0.2;
            
            float x = t*_width;

            float xLabel = x;
            
            if ((i > 0) && (i < axis->_values.size() - 1))
            {
                if (lineLabelFlag)
                {
                    float y0 = 0.0;
                    float y1 = _height;
        
                    float y0f = y0;
                    float y1f = y1;

                    y0f = _height - y0f;
                    y1f = _height - y1f;

                    // Draw a vertical line
                    x = (int)x;
                    
                    g.drawLine(x, y0f, x, y1f, axis->_lineWidth);
                }
                else
                {
                    float tx = xLabel;
                    float ty = textOffset + axis->_offsetY*_height;
                    
                    drawText(g, tx, ty,
                             FONT_SIZE, text, axis->_labelColor,
                             juce::Justification::horizontallyCentred /*| juce::Justification::bottom*/);
                }
            }

            if (!lineLabelFlag)
            {
                if (i == 0)
                {
                    float tx = xLabel + textOffset;
                    float ty = textOffset + axis->_offsetY*_height;
                    
                    // First text: align left
                    drawText(g, tx, ty, FONT_SIZE,
                             text, axis->_labelColor,
                             juce::Justification::left /*| juce::Justification::bottom*/);
                }
        
                if (i == axis->_values.size() - 1)
                {
                    float tx = xLabel - textOffset;
                    float ty = textOffset + axis->_offsetY*_height;
                    
                    // Last text: align right
                    drawText(g, tx, ty,
                             FONT_SIZE, text, axis->_labelColor,
                             juce::Justification::right /*| juce::Justification::bottom*/);
                }
            }
        }
        else
            // Vertical
        {
            float textOffset = FONT_SIZE*0.2;
            
            float y = t*_height;
            
            y += axis->_offsetY*_height; // For Ghost

            float yLabel = y;
            
            if ((i > 0) && (i < axis->_values.size() - 1))
                // First and last: don't draw axis line
            {
                if (lineLabelFlag)
                {
                    float x0 = 0.0;
                    float x1 = _width;

                    
                    y = (int)y; 
                    float yf = y;
                    
                    yf = _height - yf;
                    
                    // Draw a horizontal line
                    g.drawLine(x0, yf, x1, yf, axis->_lineWidth);
                }
                else
                {
                    float tx = textOffset + axis->_offsetX*_width;
                    float ty = yLabel;
                    
                    drawText(g, tx, ty, FONT_SIZE, text,
                             axis->_labelColor,
                             juce::Justification::left /*| juce::Justification::bottom*/);
                }
            }
            
            if (!lineLabelFlag)
            {
                if (i == 0)
                    // First text: align "top"
                {
                    float tx = textOffset + axis->_offsetX*_width;
                    float ty = yLabel + FONT_SIZE*0.75;
                    
                    drawText(g, tx, ty, FONT_SIZE, text,
                             axis->_labelColor,
                             juce::Justification::left /*| juce::Justification::bottom*/);
                }
                
                if (i == axis->_values.size() - 1)
                    // Last text: align "bottom"
                {
                    float tx = textOffset + axis->_offsetX*_width;
                    float ty = yLabel - FONT_SIZE*1.5;
                    
                    drawText(g, tx, ty, FONT_SIZE, text,
                             axis->_labelColor,
                             juce::Justification::left /*| juce::Justification::bottom*/);
                }
            }
        }
    }
    
    g.restoreState();
}

void
SpectrumViewJuce::drawCurves(juce::Graphics &g)
{
    for (int i = 0; i < _curves.size(); i++)
    {
        if (_curves[i]->_curveFill)
            drawFillCurve(g, _curves[i]);
        else
            drawLineCurve(g, _curves[i]);
    }
}

void
SpectrumViewJuce::drawText(juce::Graphics &g,
                           float x, float y,
                           float fontSize,
                           const char *text, int color[4],
                           juce::Justification align)
{
    if (strlen(text) == 0)
        return;
    
    g.saveState();
      
    g.setFont(_font.withHeight(fontSize));
    
    g.setColour(juce::Colour((juce::uint8)color[0], (juce::uint8)color[1], (juce::uint8)color[2], (juce::uint8)color[3]));

    float yf = y;
    yf = _height - y;
    
    g.drawSingleLineText(text, x, yf, align);
    
    g.restoreState();
}

void
SpectrumViewJuce::drawSeparatorY0(juce::Graphics &g)
{
    g.saveState();
    
    g.setColour(juce::Colour((juce::uint8)147, (juce::uint8)147, (juce::uint8)147, (juce::uint8)255));
    
    // Draw a vertical line at the bottom
    
    float x0 = 0;
    float x1 = _width;
    
    float y = 1.0;
    
    float yf = y;
    yf = _height - yf;
    
    g.drawLine(x0, yf, x1, yf, 2.0);
    
    g.restoreState();
}

void
SpectrumViewJuce::drawLineCurve(juce::Graphics &g, Curve *curve)
{
    bool curveUndefined = isCurveUndefined(curve->_xValues, curve->_yValues, 2);
    if (curveUndefined)
        return;
    
    g.saveState();
    
    g.setColour(juce::Colour((juce::uint8)(curve->_color[0]*255),
                             (juce::uint8)(curve->_color[1]*255),
                             (juce::uint8)(curve->_color[2]*255),
                             (juce::uint8)(curve->_color[3]*255)));
    
    juce::Path path;
            
    bool firstPoint = true;
    for (int i = 0; i < curve->_xValues.size(); i ++)
    {
        float x = curve->_xValues.data()[i];
        
        if (x >= CURVE_VALUE_UNDEFINED)
            continue;
        
        float y = curve->_yValues.data()[i];
        if (y >= CURVE_VALUE_UNDEFINED)
            continue;
        
        float yf = y;
        yf = _height - yf;
        
        if (firstPoint)
        {
            path.startNewSubPath(x, yf);
            
            firstPoint = false;
        }
        
        path.lineTo(x, yf);
    }
    
    g.strokePath(path, juce::PathStrokeType(curve->_lineWidth));
    
    g.restoreState();
}

void
SpectrumViewJuce::drawFillCurve(juce::Graphics &g, Curve *curve)
{
    bool curveUndefined = isCurveUndefined(curve->_xValues, curve->_yValues, 2);
    if (curveUndefined)
        return;
        
    // Offset used to draw the closing of the curve outside the viewport
    // Because we draw both stroke and fill at the same time
    float offset = curve->_lineWidth;
    
    g.saveState();
    
    juce::Path path;
    
    float x0 = 0.0;
    for (int i = 0; i < curve->_xValues.size(); i ++)
    {        
        float x = curve->_xValues.data()[i];
        float y = curve->_yValues.data()[i];
        
        if (x >= CURVE_VALUE_UNDEFINED)
            continue;
        
        if (y >= CURVE_VALUE_UNDEFINED)
            continue;
                
        float yf = y;
        float y1f = - offset;

        yf = _height - yf;
        y1f = _height - y1f;
        
        if (i == 0)
        {
            x0 = x;
            
            path.startNewSubPath(x0 - offset, y1f);
            path.lineTo(x - offset, yf);
        }
        
        path.lineTo(x, yf);
        
        if (i >= curve->_xValues.size() - 1)
            // Close
        {
            path.lineTo(x + offset, yf);
            path.lineTo(x + offset, y1f);
            
            path.closeSubPath();
        }
    }
    
    g.setColour(juce::Colour((juce::uint8)(curve->_fillColor[0]*255),
                             (juce::uint8)(curve->_fillColor[1]*255),
                             (juce::uint8)(curve->_fillColor[2]*255),
                             (juce::uint8)(curve->_fillColor[3]*255)));
    
    g.fillPath(path);
    
    g.setColour(juce::Colour((juce::uint8)(curve->_color[0]*255),
                              (juce::uint8)(curve->_color[1]*255),
                              (juce::uint8)(curve->_color[2]*255),
                              (juce::uint8)(curve->_color[3]*255)));
    
    g.strokePath(path, juce::PathStrokeType(curve->_lineWidth));
    
    g.restoreState();
}

void
SpectrumViewJuce::drawCurveDescriptions(juce::Graphics &g)
{
#define OFFSET_Y 4.0
    
#define DESCR_X 40.0
#define DESCR_Y0 10.0 + OFFSET_Y
    
#define DESCR_WIDTH 20
#define DESCR_Y_STEP 12
#define DESCR_SPACE 5
    
#define TEXT_Y_OFFSET 2
 
#define TEXT_Y_OFFSET2 -3
    
    int descrNum = 0;
    for (int i = 0; i < _curves.size(); i++)
    {
        Curve *curve = _curves[i];
        char *descr = curve->_description;
        if (descr == NULL)
            continue;
        
        float y = _height - (DESCR_Y0 + descrNum*DESCR_Y_STEP);
        
        g.saveState();
        
        // If line width < 0, it can be the case when we want to fill
        // a curve, but not display line over (just the fill)
        // But for the description, we need the right line width
        float prevLineWidth = curve->_lineWidth;
        if (curve->_lineWidth < 0)
            curve->_lineWidth = -curve->_lineWidth;

        // Save alpha and set it to 1
        float strokeAlpha = curve->_color[3];
        curve->_color[3] = 1.0;
        float fillAlpha = curve->_fillColor[3];
        curve->_fillColor[3] = 1.0;

        // Restore alpha
        curve->_color[3] = strokeAlpha;
        curve->_fillColor[3] = fillAlpha;
            
        curve->_lineWidth = prevLineWidth;
        
        y += TEXT_Y_OFFSET;
        
        float yf = y;
        yf = _height - yf;
        
        g.setColour(juce::Colour((juce::uint8)(curve->_color[0]*255),
                                 (juce::uint8)(curve->_color[1]*255),
                                 (juce::uint8)(curve->_color[2]*255),
                                 (juce::uint8)(curve->_color[3]*255)));
        
        g.drawLine(DESCR_X, yf, DESCR_X + DESCR_WIDTH, yf, curve->_lineWidth);
        
        drawText(g,
                 DESCR_X + DESCR_WIDTH + DESCR_SPACE,
                 y + TEXT_Y_OFFSET2,
                 FONT_SIZE, descr,
                 curve->_descrColor,
                 juce::Justification::left /*| juce::Justification::verticallyCentred*/);
        
        g.restoreState();
        
        descrNum++;
    }
}

bool
SpectrumViewJuce::isCurveUndefined(const vector<float> &x,
                               const vector<float> &y,
                               int minNumValues)
{
    if (x.size() != y.size())
        return true;
    
    int numDefinedValues = 0;
    for (int i = 0; i < x.size(); i++)
    {
        float x0 = x.data()[i];
        if (x0 >= CURVE_VALUE_UNDEFINED)
            continue;
        
        float y0 = y.data()[i];
        if (y0 >= CURVE_VALUE_UNDEFINED)
            continue;
        
        numDefinedValues++;
        
        if (numDefinedValues >= minNumValues)
            // The curve is defined
            return false;
    }
    
    return true;
}
