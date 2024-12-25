#ifndef SPECTRUM_VIEW_H
#define SPECTRUM_VIEW_H

typedef struct NVGcontext NVGcontext;

class SpectrumView
{
 public:
    SpectrumView(NVGcontext *nvgContext, int width, int height);
    virtual ~SpectrumView();

    void draw();

    void getViewSize(int *width, int *_height);
    
 protected:
    void drawAxis(bool lineLabelFlag);
    void drawAxis(Axis *axis, bool horizontal, bool lineLabelFlag);
    void drawCurves();
    void drawText(float x, float y,
                  float fontSize,
                  const char *text, int color[4],
                  int halign, int valign, float fontSizeCoeff);
    void drawSeparatorY0();
    void applyViewOrientation(const Axis &axis,
                              float *x, float *y,
                              int *labelHAlign);
    float convertToBoundsY(float t);
    float ConvertToAxisBounds(Axis *axis, float t);
    void drawLineCurve(Curve *curve);
    void drawFillCurve(Curve *curve);
    bool isCurveUndefined(const vector<float> &x,
                          const vector<float> &y,
                          int minNumValues);
    void setCurveDrawStyle(Curve *curve);
};

#endif
