#include "SpectrumView.h"

#define FONT "Roboto-Bold"
#define FONT_SIZE 14.0

SpectrumView::SpectrumView(NVGcontext *nvgContext, int width, int height)
{
    _nvgContext = nvgContext;
    _width = width;
    _height = height;
}

SpectrumView::~SpectrumView() {}

SpectrumView::draw()
{
    drawAxis(true);

    nvgSave(_nvgContext);    
    drawCurves();
    nvgRestore(_nvgContext);

    drawAxis(false);
    
    drawCurveDescriptions();

    drawSeparatorY0();
}

void
SpectrumView::getViewSize(int *width, int *_height)
{
    *width = _width;
    *height = _height;

void
SpectrumView::drawAxis(bool lineLabelFlag)
{
    if (_hAxis != NULL)
        drawAxis(_hAxis, true, lineLabelFlag);
    
    if (_vAxis != NULL)
        drawAxis(_vAxis, false, lineLabelFlag);
}

void
SpectrumView::drawAxis(Axis *axis, bool horizontal, bool lineLabelFlag)
{
    nvgSave(_nvgContext);

    nvgStrokeWidth(_nvgContext, axis->_lineWidth);
    
    nvgStrokeColor(_nvgContext, nvgRGBA(axis->_color[0], axis->_color[1],
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
            xLabel += axis->_offsetPixels;
            
            if (((i > 0) && (i < axis->_values.size() - 1)) ||
                !axis->_alignBorderLabels)
            {
                if (lineLabelFlag)
                {
                    float y0 = 0.0;
                    float y1 = height;
        
                    float y0f = y0;
                    float y1f = y1;

                    y0f = height - y0f;
                    y1f = height - y1f;

                    // Draw a vertical line
                    nvgBeginPath(_nvgContext);

                    if (axis->_alignToScreenPixels)
                        x = (int)x;
                    
                    nvgMoveTo(_nvgContext, x, y0f);
                    nvgLineTo(_nvgContext, x, y1f);
    
                    nvgStroke(_nvgContext);
                }
                else
                {
                    float tx = xLabel;
                    float ty = textOffset + axis->_offsetY*height;

                    int halign = NVG_ALIGN_CENTER;
                    
                    applyViewOrientation(*axis, &tx, &ty, &halign);
                    
                    drawText(tx, ty,
                             FONT_SIZE, text, axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM,
                             axis->_fontSizeCoeff);
                }
            }

            if (!lineLabelFlag && axis->_alignBorderLabels)
            {
                if (i == 0)
                {
                    float tx = xLabel + textOffset;
                    float ty = textOffset + axis->_offsetY*height;

                    int halign = NVG_ALIGN_LEFT;
                    
                    applyViewOrientation(*axis, &tx, &ty, &halign);
                    
                    // First text: aligne left
                    drawText(tx, ty, FONT_SIZE,
                             text, axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM,
                             axis->_fontSizeCoeff);
                }
        
                if (i == axis->_values.size() - 1)
                {
                    float tx = xLabel - textOffset;
                    float ty = textOffset + axis->_offsetY*height;

                    int halign = NVG_ALIGN_RIGHT;
                    
                    applyViewOrientation(*axis, &tx, &ty, &halign);
                    
                    // Last text: align right
                    drawText(tx, ty,
                             FONT_SIZE, text, axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM,
                             axis->_fontSizeCoeff);
                }
            }
        }
        else
            // Vertical
        {
            float textOffset = FONT_SIZE*0.2;
                
            t = convertToBoundsY(t);

            t = convertToAxisBounds(axis, t);
            
            float y = t*_height;
            
            y += axis->_offsetY*height; // For Ghost

            float yLabel = y;
            yLabel += axis->_offsetPixels;
            
            if (((i > 0) && (i < axis->_values.size() - 1)) ||
                !axis->_alignBorderLabels)
                // First and last: don't draw axis line
            {
                if (lineLabelFlag)
                {
                    float x0 = 0.0;
                    float x1 = width;

                    if (axis->_alignToScreenPixels)
                        y = (int)y;
                    
                    float yf = y;
                    
                    yf = height - yf;
                    
                    // Draw a horizontal line
                    nvgBeginPath(_nvgContext);
                    
                    nvgMoveTo(_nvgContext, x0, yf);
                    nvgLineTo(_nvgContext, x1, yf);
                    
                    nvgStroke(_nvgContext);
                }
                else
                {
                    int align = NVG_ALIGN_LEFT;
                    if (axis->_alignTextRight)
                        align = NVG_ALIGN_RIGHT;
                    
                    float tx = textOffset + axis->_offsetX*width;
                    float ty = yLabel;

                    int halign = align | NVG_ALIGN_MIDDLE;
                    
                    applyViewOrientation(*axis, &tx, &ty, &halign);
                    
                    grawText(tx, ty, FONT_SIZE, text,
                             axis->_labelColor,
                             halign,
                             NVG_ALIGN_BOTTOM,
                             axis->_fontSizeCoeff);
                }
            }
            
            if (!lineLabelFlag && axis->_alignBorderLabels)
            {
                if (i == 0)
                    // First text: align "top"
                {
                    float tx = textOffset + axis->_affsetX*width;
                    float ty = yLabel + FONT_SIZE*0.75;

                    int halign = NVG_ALIGN_LEFT;
                    applyViewOrientation(*axis, &tx, &ty, &halign);
                    
                    drawText(tx, ty, FONT_SIZE, text,
                             axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM,
                             axis->_fontSizeCoeff);
                }
                
                if (i == axis->_values.size() - 1)
                    // Last text: align "bottom"
                {
                    float tx = textOffset + axis->_offsetX*width;
                    float ty = yLabel - FONT_SIZE*1.5;

                    int halign = NVG_ALIGN_LEFT;
                    applyViewOrientation(*axis, &tx, &ty, &halign);
                    
                    drawText(tx, ty, FONT_SIZE, text,
                             axis->_labelColor,
                             halign, NVG_ALIGN_BOTTOM,
                             axis->_fontSizeCoeff);
                }
            }
        }
    }
    
    nvgRestore(_nvgContext);
}

void
SpectrumView::drawCurves()
{
    for (int i = 0; i < _curves.size(); i++)
    {
        if (_curves[i]->_curveFill)
            drawFillCurve(_curves[i]);
        else
            drawLineCurve(_curves[i]);
    }
}

void
SpectrumView::drawText(float x, float y,
                       float fontSize,
                       const char *text, int color[4],
                       int halign, int valign, float fontSizeCoeff)
{
    if (strlen(text) == 0)
        return;
    
    nvgSave(_nvgContext);
        
    nvgFontSize(_nvgContext, fontSize*fontSizeCoeff);
	nvgFontFace(_nvgContext, FONT);
    nvgFontBlur(_nvgContext, 0);
	nvgTextAlign(_nvgContext, halign | valign);
    
    nvgFillColor(_nvgContext, nvgRGBA(color[0], color[1], color[2], color[3]));

    float yf = y;
    yf = _height - y;
    
	nvgText(_nvgContext, x, yf, text, NULL);
    
    nvgRestore(vg);
}

void
SpectrumView::drawSeparatorY0()
{
    if (!_separatorY0)
        return;
    
    nvgSave(_nvgContext);
    nvgStrokeWidth(_nvgContext, _sepY0LineWidth);
    
    nvgStrokeColor(_nvgContext, nvgRGBA(_sepY0Color[0], _sepY0Color[1], _sepY0Color[2], _sepY0Color[3]));
    
    // Draw a vertical line ath the bottom
    nvgBeginPath(_nvgContext);
    
    float x0 = 0;
    float x1 = _width;
    
    float y = _sepY0LineWidth/2.0;
    
    float yf = y;
    yf = height - yf;
    
    nvgMoveTo(_nvgContext, x0, yf);
    nvgLineTo(_nvgContext, x1, yf);
                    
    nvgStroke(_nvgContext);
    
    nvgRestore(_nvgContext);
}

// Apply -M_PI/2 rotation on coordinates
void
SpectrumView::applyViewOrientation(const Axis &axis,
                                   float *x, float *y,
                                   int *labelHAlign)
{
    if (axis._viewOrientation == Axis::VERTICAL)
    {
        // Rotate 90 degrees
        x = x / _width;
        y = y / _height;
    
        float  tmp = x;
        x = y;
        y = tmp;
    
        x = _width;
        y = _height;
    
        x = _width - x;
    }
    
    if (axis._forceLabelHAlign >= 0)
        *labelHAlign = axis._forceLabelHAlign;
}

float
SpectrumView::convertToBoundsY(float t)
{
    // Rescale
    t *= (_bounds[3] - _bounds[1]);
    t += (1.0 - _bounds[3]);
    
    // Clip
    if (t < 0.0)
        t = 0.0;
    if (t > 1.0)
        t = 1.0;
    
    return t;
}

float
SpectrumView::ConvertToAxisBounds(Axis *axis, float t)
{
    // Rescale
    t *= (axis->_bounds[1] - axis->_bounds[0]);
    t += axis->_bounds[0];
    
    // Clip
    if (t < 0.0)
        t = 0.0;
    if (t > 1.0)
        t = 1.0;
    
    return t;
}

void
SpectrumView::drawLineCurve(Curve *curve)
{        
    bool curveUndefined = isCurveUndefined(curve->_xValues, curve->_yValues, 2);
    if (curveUndefined)
        return;
    
    nvgSave(_nvgContext);
    
    setCurveDrawStyle(curve);
    
    nvgBeginPath(_nvgContext);
            
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
        yf = height - yf;
        
        if (firstPoint)
        {
            nvgMoveTo(_nvgContext, x, yf);
            
            firstPoint = false;
        }
        
        nvgLineTo(_nvgContext, x, yf);
    }
    
    nvgStroke(_nvgContext);
    nvgRestore(_nvgContext);
}

void
SpectrumView::drawFillCurve(Curve *curve)
{
    bool curveUndefined = isCurveUndefined(curve->_xValues, curve->_yValues, 2);
    if (curveUndefined)
        return;
    
    // Offset used to draw the closing of the curve outside the viewport
    // Because we draw both stroke and fill at the same time
    float offset = curve->_lineWidth;
        
    nvgSave(_nvgContext);
    
    nvgBeginPath(_nvgContext);
    
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
        float y1f = curve->_curveFillOriginY*_height - offset;

        yf = height - yf;
        y1f = height - y1f;
        
        if (i == 0)
        {
            x0 = x;
            
            nvgMoveTo(_nvgContext, x0 - offset, y1f);
            nvgLineTo(_nvgContext, x - offset, yf);
        }
        
        nvgLineTo(_nvgContext, x, yf);
        
        if (i >= curve->_xValues.GetSize() - 1)
            // Close
        {
            nvgLineTo(_nvgContext, x + offset, yf);
            nvgLineTo(_nvgContext, x + offset, y1f);
            
            nvgClosePath(_nvgContext);
        }
    }
    
    nvgFillColor(_nvgContext, nvgRGBA(curve->_fillColor[0], curve->_fillColor[1],
                              curve->_fillColor[2], curve->_fillColor[3]));
	nvgFill(_nvgContext);
    
    nvgStrokeColor(_nvgContext, nvgRGBA(curve->_strokeColor[0], curve->_strokeColor[1],
                                curve->_strokeColor[2], curve->_strokeColor[3]));
    
    nvgStrokeWidth(_nvgContext, curve->_lineWidth);
    nvgStroke(_nvgContext);

    nvgRestore(_nvgContext);
}

bool
SpectrumView::isCurveUndefined(const vector<float> &x,
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
SpectrumView::setCurveDrawStyle(Curve *curve)
{
    nvgStrokeWidth(_nvgContext, curve->_lineWidth);
    
    nvgStrokeColor(mVg, nvgRGBA(curve->_color[0]*255, curve->_color[1]*255,
                                curve->_color[2]*255, curve->_color[3]*255));
    
    nvgFillColor(mVg, nvgRGBA(curve->_fillColor[0]*255, curve->_fillColor[1]*255,
                                curve->_fillColor[2]*255, curve->_fillColor[3]*255));
}
