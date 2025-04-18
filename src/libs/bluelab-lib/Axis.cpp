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

#include "Scale.h"

#include "Axis.h"

Axis::Axis()
{    
    for (int i = 0; i < 4; i++)
        _color[i] = 0;
    
    for (int i = 0; i < 4; i++)
        _labelColor[i] = 0;
    
    _offsetX = 0.0;
    _offsetY = 0.0;
    
    _scaleType = Scale::LINEAR;
    _minVal = 0.0;
    _maxVal = 1.0;
    
    _lineWidth = 1.0;
    
    _scale = new Scale();
}

Axis::~Axis()
{
    delete _scale;
}

void
Axis::initHAxis(Scale::Type scale,
                float minX, float maxX,
                int axisColor[4], int axisLabelColor[4],
                float lineWidth,
                float offsetY)
{   
    _offsetX = 0.0;
    _offsetY = offsetY;
    
    _scaleType = scale;
    _minVal = minX;
    _maxVal = maxX;
    
    init(axisColor, axisLabelColor, lineWidth);
}

void
Axis::initVAxis(Scale::Type scale,
                float minY, float maxY,
                int axisColor[4], int axisLabelColor[4],
                float lineWidth,
                float offsetX, float offsetY)
{
    _offsetX = offsetX;
    _offsetY = offsetY;
    
    _scaleType = scale;
    _minVal = minY;
    _maxVal = maxY;
    
    init(axisColor, axisLabelColor, lineWidth);
}

void
Axis::setMinMaxValues(float minVal, float maxVal)
{
    _minVal = minVal;
    _maxVal = maxVal;
}

void
Axis::setData(char *data[][2], int numData)
{
    _values.resize(numData);

    Data aData;
    string text;

    float rangeInv = 1.0/(_maxVal - _minVal);
    
    // Copy data
    for (int i = 0; i < numData; i++)
    {
        float val = atof(data[i][0]);
        float t = (val - _minVal)*rangeInv;
        
        t = _scale->applyScale(_scaleType, t, _minVal, _maxVal);
        
        text = data[i][1];
        
        aData._t = t;
        aData._text = text;
        
        _values[i] = aData;
    }
}

void
Axis::setScaleType(Scale::Type scaleType)
{
    _scaleType = scaleType;
}

void
Axis::init(int color[4], int labelColor[4], float lineWidth)
{
    for (int i = 0; i < 4; i++)
    {
        _color[i] = color[i];
        _labelColor[i] = labelColor[i];
    }
    
    _lineWidth = lineWidth;
}
