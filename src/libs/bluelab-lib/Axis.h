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

#ifndef AXIS_H
#define AXIS_H

#include <string>
using namespace std;

#include "Scale.h"

class Axis
{
public:
    Axis();
    
    virtual ~Axis();
    
    void initHAxis(Scale::Type scale,
                   float minX, float maxX,
                   int axisColor[4], int axisLabelColor[4],
                   float lineWidth,
                   float offsetY = 0.0);
    
    void initVAxis(Scale::Type scale,
                   float minY, float maxY,
                   int axisColor[4], int axisLabelColor[4],
                   float lineWidth,
                   float offsetX = 0.0, float offsetY = 0.0);
    
    void setMinMaxValues(float minVal, float maxVal);
    
    void setData(char *data[][2], int numData);
    
    void setAlignToScreenPixels(bool flag);

    void setOffsetPixels(float offsetPixels);

    void setScaleType(Scale::Type scaleType);
    
protected:
    void init(int axisColor[4],
              int axisLabelColor[4],
              float lineWidth);

    friend class SpectrumView;
        
    typedef struct
    {
        float _t;
        string _text;
    } Data;
    
    vector<Data> _values;
    
    Scale::Type _scaleType;
    float _minVal;
    float _maxVal;
    
    int _color[4];
    int _labelColor[4];
    
    // To be able to display the axis on the right
    float _offsetX;
    float _offsetY;
    
    float _lineWidth;

    Scale *_scale;
};

#endif
