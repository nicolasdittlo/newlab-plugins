#ifndef AXIS_H
#define AXIS_H

#include "Scale.h"

class Axis
{
public:
    enum ViewOrientation
    {
        HORIZONTAL = 0,
        VERTICAL
    };
    
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

    // Align the first and last labels to the borders of the graph?
    void setAlignBorderLabels(bool flag);

    // View orientation (for Panogram)
    void setViewOrientation(ViewOrientation orientation);
    
    // Set to -1 to not force (and keep th default)
    void setForceLabelHAlign(int align);

    void setScaleType(Scale::Type scaleType);
    
protected:
    void init(int axisColor[4],
              int axisLabelColor[4],
              float lineWidth);
    
    typedef struct
    {
        float mT;
        string mText;
    } AxisData;
    
    
    vector<AxisData> _values;
    
    Scale::Type mScaleType;
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

    bool _alignBorderLabels;

    ViewOrientation _viewOrientation;
    
    int _forceLabelHAlign;
};

#endif
