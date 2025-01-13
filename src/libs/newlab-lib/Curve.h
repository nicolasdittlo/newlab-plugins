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

#ifndef CURVE_H
#define CURVE_H

#include <vector>
using namespace std;

#include "Scale.h"

#define CURVE_VALUE_UNDEFINED 1e16

class Curve
{
public:
    Curve(int numValues);
    
    virtual ~Curve();
    
    void setViewSize(float width, float height);
    
    void setDescription(const char *description, int descrColor[4]);

    void setValues(const vector<float> &values,
                   bool applyXScale = true, bool applyYScale = true);
    void clearValues();

    void setXScale(Scale::Type scale,
                   float minX = 0.0, float maxX = 1.0);
    
    void setYScale(Scale::Type scale,
                   float minY = -120.0,
                   float maxY = 0.0);

    void getYScale(Scale::Type *scale, float *minY, float *maxY);
    
    void setColor(int r, int g, int b);
    void setLineWidth(float lineWidth);
    
    void setFill(bool flag);
    void setFillColor(int r, int g, int b, int a);

    void setAlpha(float alpha);
    void setFillAlpha(float alpha);
    
 protected:
    friend class SpectrumView;
    
    // Description
    char *_description;
    int _descrColor[4];
    
    // Scale
    Scale::Type _xScale;
    float _minX;
    float _maxX;
    
    Scale::Type _yScale;
    float _minY;
    float _maxY;
    
    float _color[4];

    bool _curveFill;
    float _fillColor[4];
    
    float _lineWidth;
        
    int _numValues;
    vector<float> _xValues;
    vector<float> _yValues;
    
    float _viewSize[2];

    friend class SmoothCurveDB;
    Scale *_scale;
};

#endif

