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

#include <stdio.h>
#include <string.h>

#include "Utils.h"
#include "Curve.h"

#define SET_COLOR_FROM_INT(__COLOR__, __R__, __G__, __B__, __A__)   \
    __COLOR__[0] = ((float)__R__)/255.0;                            \
    __COLOR__[1] = ((float)__G__)/255.0;                            \
    __COLOR__[2] = ((float)__B__)/255.0;                            \
    __COLOR__[3] = ((float)__A__)/255.0;

Curve::Curve(int numValues)
{
    _description = NULL;

    _scale  = new Scale();
    
    _yScale = Scale::LINEAR;
    _minY = 0.0;
    _maxY = 1.0;
    
    SET_COLOR_FROM_INT(_color, 0, 0, 0, 255);
    
    _xScale = Scale::LINEAR;
    _minX = 0.0;
    _maxX = 1.0;
    
    _lineWidth = 1.5;
    
    _curveFill = false;
    
    SET_COLOR_FROM_INT(_fillColor, 0, 0, 0, 255);
    
    _numValues = numValues;
    _xValues.resize(_numValues);
    _yValues.resize(_numValues);
        
    clearValues();
    
    _viewSize[0] = 256;
    _viewSize[1] = 256;
}

Curve::~Curve()
{
    if (_description != NULL)
        delete []_description;

    delete _scale;
}

void
Curve::setViewSize(float width, float height)
{
    _viewSize[0] = width;
    _viewSize[1] = height;
}

void
Curve::setDescription(const char *description, int descrColor[4])
{
#define DESCRIPTION_MAX_SIZE 256
    
    if (_description != NULL)
        delete []_description;
    
    _description = new char[DESCRIPTION_MAX_SIZE];
    memset(_description, 0, DESCRIPTION_MAX_SIZE*sizeof(char));
    
    sprintf(_description, "%s", description);
    
    for (int i = 0; i < 4; i++)
        _descrColor[i] = descrColor[i];
}

void
Curve::clearValues()
{
    Utils::fillValue(&_xValues, (float)CURVE_VALUE_UNDEFINED);
    Utils::fillValue(&_yValues, (float)CURVE_VALUE_UNDEFINED);
}

void
Curve::setYScale(Scale::Type scale, float minY, float maxY)
{
    _yScale = scale;
    _minY = minY;
    _maxY = maxY;
}

void
Curve::getYScale(Scale::Type *scale, float *minY, float *maxY)
{
    *scale = _yScale;
    *minY = _minY;
    *maxY = _maxY;
}

void
Curve::setXScale(Scale::Type scale, float minX, float maxX)
{
    _xScale = scale;
    _minX = minX;
    _maxX = maxX;
}

void
Curve::setColor(int r, int g, int b)
{
    SET_COLOR_FROM_INT(_color, r, g, b, 255);
}

void
Curve::setLineWidth(float lineWidth)
{
    _lineWidth = lineWidth;
}

void
Curve::setFill(bool flag)
{
    _curveFill = flag;
}

void
Curve::setFillColor(int r, int g, int b, int a)
{
    SET_COLOR_FROM_INT(_fillColor, r, g, b, a);
}

void
Curve::setValues(const vector<float> &values,
                 bool applyXScale, bool applyYScale)
{
    // Normalize, then adapt to the graph
    int width = _viewSize[0];
    int height = _viewSize[1];

    // X
    _xValues.resize(values.size());
    int numXValues = _xValues.size();
    float *xValuesData = _xValues.data();

    float t = 0.0;
    float tincr = 0.0;
    if (_xValues.size() > 1)
        tincr = 1.0/(_xValues.size() - 1);
    for (int i = 0; i < numXValues; i++)
    {
        xValuesData[i] = t;
        t += tincr;
    }

    if (applyXScale)
        _scale->applyScaleForEach(_xScale, &_xValues, _minX, _maxX);

    Utils::multValue(&_xValues, (float)width);
    
    // Y
    _yValues = values;
    int numYValues = _yValues.size();
    float *yValuesData = _yValues.data();
    
    if (applyYScale)
    {
        if (_yScale != Scale::LINEAR)
            _scale->applyScaleForEach(_yScale, &_yValues, _minY, _maxY);
        else
            _scale->applyScaleForEach(Scale::NORMALIZED, &_yValues, _minY, _maxY);
    }
    
    Utils::multValue(&_yValues, height);
}
