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
    
    _fontSizeCoeff = 1.0;
    
    _scaleType = Scale::LINEAR;
    _minVal = 0.0;
    _maxVal = 1.0;
    
    _alignTextRight = false;
    
    _alignRight = false;
    
    _lineWidth = 1.0;
    
    _scale = new Scale();

    // align by default
    _alignToScreenPixels = true;

    _offsetPixels = 0.0;

    _alignBorderLabels = true;

    _viewOrientation = HORIZONTAL;
    _forceLabelHAlign = -1;
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
                float offsetY,
                float fontSizeCoeff)
{   
    _offsetX = 0.0;
    _offsetY = offsetY;
    
    _fontSizeCoeff = fontSizeCoeff;
    
    _scaleType = scale;
    _minVal = minX;
    _maxVal = maxX;
    
    _alignTextRight = false;
    _alignRight = true;
    
    init(axisColor, axisLabelColor,
         axisOverlayColor, axisLinesOverlayColor, lineWidth);
}

void
Axis::initVAxis(Scale::Type scale,
                float minY, float maxY,
                int axisColor[4], int axisLabelColor[4],
                float lineWidth,
                float offsetX, float offsetY,
                float fontSizeCoeff, bool alignTextRight,
                bool alignRight)
{
    _offsetX = offsetX;
    _offsetY = offsetY;
    
    _fontSizeCoeff = fontSizeCoeff;
    
    _scaleType = scale;
    _minVal = minY;
    _maxVal = maxY;
    
    _alignTextRight = alignTextRight;
    _alignRight = alignRight;
    
    init(axisColor, axisLabelColor,
         axisOverlayColor, axisLinesOverlayColor,
         lineWidth);
}

void
Axis::SetMinMaxValues(float minVal, float maxVal)
{
    mMinVal = minVal;
    mMaxVal = maxVal;
    
    NotifyGraph();
}

void
Axis::setData(char *data[][2], int numData)
{
    _values.resize(numData);

    AxisData aData;
    string text;

    float rangeInv = 1.0/(_maxVal - _minVal);
    
    // Copy data
    for (int i = 0; i < numData; i++)
    {
        float val = atof(data[i][0]);
        float t = (val - _minVal)*rangeInv;
        
        t = _scale->ApplyScale(_scaleType, t, mMinVal, mMaxVal);
        
        text = data[i][1];
        
        aData._t = t;
        aData._text = text;
        
        _values[i] = aData;
    }
}

void
Axis::setAlignToScreenPixels(bool flag)
{
    _alignToScreenPixels = flag;
}

void
Axis::setViewOrientation(ViewOrientation orientation)
{
    _viewOrientation = orientation;
}

void
Axis::setForceLabelHAlign(int align)
{
    _forceLabelHAlign = align;
}

void
Axis::setScaleType(Scale::Type scaleType)
{
    _scaleType = scaleType;
}

void
Axis::setOffsetPixels(BL_FLOAT offsetPixels)
{
    _offsetPixels = offsetPixels;
}

void
Axis::setAlignBorderLabels(bool flag)
{
    _alignBorderLabels = flag;
}

void
Axis::init(int axisColor[4], int axisLabelColor[4],
           float lineWidth)
{
    for (int i = 0; i < 4; i++)
    {
        _color[i] = _axisColor[i];
        _labelColor[i] = labelColor[i];
    }
    
    _lineWidth = lineWidth;
}
