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

#include <nanovg.h>

#include "Axis.h"
#include "Curve.h"
#include "SpectrumViewNVG.h"

#define FONT "Roboto-Bold"
#define FONT_SIZE 12.0

SpectrumViewNVG::SpectrumViewNVG()
{
    _width = 256;
    _height = 256;
}

SpectrumViewNVG::~SpectrumViewNVG() {}

void
SpectrumViewNVG::draw(NVGcontext *nvgContext)
{
    drawAxis(nvgContext, true);

    nvgSave(nvgContext);    
    drawCurves(nvgContext);
    nvgRestore(nvgContext);

    drawAxis(nvgContext, false);
    
    drawCurveDescriptions(nvgContext);

    drawSeparatorY0(nvgContext);
}

void
SpectrumViewNVG::drawAxis(NVGcontext *nvgContext, bool lineLabelFlag)
{
    if (_hAxis != NULL)
        drawAxis(nvgContext, _hAxis, true, lineLabelFlag);
    
    if (_vAxis != NULL)
        drawAxis(nvgContext, _vAxis, false, lineLabelFlag);
}

void
SpectrumViewNVG::drawAxis(NVGcontext *nvgContext, Axis *axis, bool horizontal, bool lineLabelFlag)
{
    nvgSave(nvgContext);

    nvgStrokeWidth(nvgContext, axis->_lineWidth);
    
    nvgStrokeColor(nvgContext, nvgRGBA(axis->_color[0], axis->_color[1],
                                        axis->_color[2], axis->_color[3]));
    
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
                    nvgBeginPath(nvgContext);

                    x = (int)x;
                    
                    nvgMoveTo(nvgContext, x, y0f);
                    nvgLineTo(nvgContext, x, y1f);
    
                    nvgStroke(nvgContext);
                }
                else
                {
                    float tx = xLabel;
                    float ty = textOffset + axis->_offsetY*_height;

                    int halign = NVG_ALIGN_CENTER;
                    
                    drawText(nvgContext, tx, ty,
                             FONT_SIZE, text, axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM);
                }
            }

            if (!lineLabelFlag)
            {
                if (i == 0)
                {
                    float tx = xLabel + textOffset;
                    float ty = textOffset + axis->_offsetY*_height;

                    int halign = NVG_ALIGN_LEFT;
                    
                    // First text: aligne left
                    drawText(nvgContext, tx, ty, FONT_SIZE,
                             text, axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM);
                }
        
                if (i == axis->_values.size() - 1)
                {
                    float tx = xLabel - textOffset;
                    float ty = textOffset + axis->_offsetY*_height;

                    int halign = NVG_ALIGN_RIGHT;
                    
                    // Last text: align right
                    drawText(nvgContext, tx, ty,
                             FONT_SIZE, text, axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM);
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
                    nvgBeginPath(nvgContext);
                    
                    nvgMoveTo(nvgContext, x0, yf);
                    nvgLineTo(nvgContext, x1, yf);
                    
                    nvgStroke(nvgContext);
                }
                else
                {
                    int align = NVG_ALIGN_LEFT;
                    
                    float tx = textOffset + axis->_offsetX*_width;
                    float ty = yLabel;

                    int halign = align | NVG_ALIGN_MIDDLE;
                    
                    drawText(nvgContext, tx, ty, FONT_SIZE, text,
                             axis->_labelColor,
                             halign,
                             NVG_ALIGN_BOTTOM);
                }
            }
            
            if (!lineLabelFlag)
            {
                if (i == 0)
                    // First text: align "top"
                {
                    float tx = textOffset + axis->_offsetX*_width;
                    float ty = yLabel + FONT_SIZE*0.75;

                    int halign = NVG_ALIGN_LEFT;
                    
                    drawText(nvgContext, tx, ty, FONT_SIZE, text,
                             axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM);
                }
                
                if (i == axis->_values.size() - 1)
                    // Last text: align "bottom"
                {
                    float tx = textOffset + axis->_offsetX*_width;
                    float ty = yLabel - FONT_SIZE*1.5;

                    int halign = NVG_ALIGN_LEFT;
                    
                    drawText(nvgContext, tx, ty, FONT_SIZE, text,
                             axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM);
                }
            }
        }
    }
    
    nvgRestore(nvgContext);
}

void
SpectrumViewNVG::drawCurves(NVGcontext *nvgContext)
{
    for (int i = 0; i < _curves.size(); i++)
    {
        if (_curves[i]->_curveFill)
            drawFillCurve(nvgContext, _curves[i]);
        else
            drawLineCurve(nvgContext, _curves[i]);
    }
}

void
SpectrumViewNVG::drawText(NVGcontext *nvgContext,
                       float x, float y,
                       float fontSize,
                       const char *text, int color[4],
                       int halign, int valign)
{
    if (strlen(text) == 0)
        return;
    
    nvgSave(nvgContext);
        
    nvgFontSize(nvgContext, fontSize);
	nvgFontFace(nvgContext, FONT);
    nvgFontBlur(nvgContext, 0);
	nvgTextAlign(nvgContext, halign | valign);
    
    nvgFillColor(nvgContext, nvgRGBA(color[0], color[1], color[2], color[3]));

    float yf = y;
    yf = _height - y;
    
	nvgText(nvgContext, x, yf, text, NULL);
    
    nvgRestore(nvgContext);
}

void
SpectrumViewNVG::drawSeparatorY0(NVGcontext *nvgContext)
{
    nvgSave(nvgContext);
    nvgStrokeWidth(nvgContext, 2.0);
    
    nvgStrokeColor(nvgContext, nvgRGBA(147, 147, 147, 255));
    
    // Draw a vertical line ath the bottom
    nvgBeginPath(nvgContext);
    
    float x0 = 0;
    float x1 = _width;
    
    float y = 1.0;
    
    float yf = y;
    yf = _height - yf;
    
    nvgMoveTo(nvgContext, x0, yf);
    nvgLineTo(nvgContext, x1, yf);
                    
    nvgStroke(nvgContext);
    
    nvgRestore(nvgContext);
}

void
SpectrumViewNVG::drawLineCurve(NVGcontext *nvgContext, Curve *curve)
{
    bool curveUndefined = isCurveUndefined(curve->_xValues, curve->_yValues, 2);
    if (curveUndefined)
        return;
    
    nvgSave(nvgContext);
    
    setCurveDrawStyle(nvgContext, curve);
    
    nvgBeginPath(nvgContext);
            
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
            nvgMoveTo(nvgContext, x, yf);
            
            firstPoint = false;
        }
        
        nvgLineTo(nvgContext, x, yf);
    }
    
    nvgStroke(nvgContext);
    nvgRestore(nvgContext);
}

void
SpectrumViewNVG::drawFillCurve(NVGcontext *nvgContext, Curve *curve)
{
    bool curveUndefined = isCurveUndefined(curve->_xValues, curve->_yValues, 2);
    if (curveUndefined)
        return;
        
    // Offset used to draw the closing of the curve outside the viewport
    // Because we draw both stroke and fill at the same time
    float offset = curve->_lineWidth;
    
    nvgSave(nvgContext);

    setCurveDrawStyle(nvgContext, curve);
    
    nvgBeginPath(nvgContext);
    
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
            
            nvgMoveTo(nvgContext, x0 - offset, y1f);
            nvgLineTo(nvgContext, x - offset, yf);
        }
        
        nvgLineTo(nvgContext, x, yf);
        
        if (i >= curve->_xValues.size() - 1)
            // Close
        {
            nvgLineTo(nvgContext, x + offset, yf);
            nvgLineTo(nvgContext, x + offset, y1f);
            
            nvgClosePath(nvgContext);
        }
    }
    
	nvgFill(nvgContext);
    nvgStroke(nvgContext);

    nvgRestore(nvgContext);
}

void
SpectrumViewNVG::drawCurveDescriptions(NVGcontext *nvgContext)
{
#define OFFSET_Y 4.0
    
#define DESCR_X 40.0
#define DESCR_Y0 10.0 + OFFSET_Y
    
#define DESCR_WIDTH 20
#define DESCR_Y_STEP 12
#define DESCR_SPACE 5
    
#define TEXT_Y_OFFSET 2
    
    int descrNum = 0;
    for (int i = 0; i < _curves.size(); i++)
    {
        Curve *curve = _curves[i];
        char *descr = curve->_description;
        if (descr == NULL)
            continue;
        
        float y = _height - (DESCR_Y0 + descrNum*DESCR_Y_STEP);
        
        nvgSave(nvgContext);
        
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
        
        setCurveDrawStyle(nvgContext, curve);

        // Restore alpha
        curve->_color[3] = strokeAlpha;
        curve->_fillColor[3] = fillAlpha;
            
        curve->_lineWidth = prevLineWidth;
        
        y += TEXT_Y_OFFSET;
        
        float yf = y;
        yf = _height - yf;
        
        nvgBeginPath(nvgContext);
        
        nvgMoveTo(nvgContext, DESCR_X, yf);
        nvgLineTo(nvgContext, DESCR_X + DESCR_WIDTH, yf);
        
        nvgStroke(nvgContext);
        
        drawText(nvgContext,
                 DESCR_X + DESCR_WIDTH + DESCR_SPACE,
                 y,
                 FONT_SIZE, descr,
                 curve->_descrColor,
                 NVG_ALIGN_LEFT, NVG_ALIGN_MIDDLE);
        
        nvgRestore(nvgContext);
        
        descrNum++;
    }
}

bool
SpectrumViewNVG::isCurveUndefined(const vector<float> &x,
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

void
SpectrumViewNVG::setCurveDrawStyle(NVGcontext *nvgContext, Curve *curve)
{
    nvgLineJoin(nvgContext, NVG_BEVEL);
    
    nvgStrokeWidth(nvgContext, curve->_lineWidth);
    
    nvgStrokeColor(nvgContext, nvgRGBA(curve->_color[0]*255, curve->_color[1]*255,
                                       curve->_color[2]*255, curve->_color[3]*255));
    
    nvgFillColor(nvgContext, nvgRGBA(curve->_fillColor[0]*255, curve->_fillColor[1]*255,
                                     curve->_fillColor[2]*255, curve->_fillColor[3]*255));
}
