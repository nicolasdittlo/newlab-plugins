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
                   float offsetY = 0.0,
                   float fontSizeCoeff = 1.0);
    
    void initVAxis(Scale::Type scale,
                   float minY, float maxY,
                   int axisColor[4], int axisLabelColor[4],
                   float lineWidth,
                   float offsetX = 0.0, float offsetY = 0.0,
                   float fontSizeCoeff = 1.0,
                   bool alignTextRight = false,
                   bool alignRight = true);
    
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
    
    float _fontSizeCoeff;
    
    bool _alignTextRight;
    bool _alignRight;
    
    float _lineWidth;

    Scale *_scale;

    bool _alignToScreenPixels;

    // For time axis scroll
    float _offsetPixels;
};

#endif
