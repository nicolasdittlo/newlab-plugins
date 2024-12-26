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
    
    void setFillColor(int r, int g, int b);

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
    
    float _lineWidth;
       
    bool _curveFill;

    float _fillColor[4];
    float _fillAlpha;
    

    bool _fillColorSet;
        
    int _numValues;
    vector<float> _xValues;
    vector<float> _yValues;
    
    float _viewSize[2];

    friend class SmoothCurveDB;
    Scale *_scale;
};

#endif

