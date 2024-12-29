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

#ifndef SPECTRUM_VIEW_H
#define SPECTRUM_VIEW_H

#include <vector>
using namespace std;

typedef struct NVGcontext NVGcontext;

class Axis;
class Curve;
class SpectrumView
{
 public:
    SpectrumView();
    virtual ~SpectrumView();

    void setHAxis(Axis *axis);
    void setVAxis(Axis *axis);

    void addCurve(Curve *curve);
    
    void draw(NVGcontext *nvgContext);

    void setViewSize(int width, int height);
    void getViewSize(int *width, int *_height);
    
 protected:
    void drawAxis(NVGcontext *nvgContext, bool lineLabelFlag);
    void drawAxis(NVGcontext *nvgContext, Axis *axis, bool horizontal, bool lineLabelFlag);
    void drawCurves(NVGcontext *nvgContext);
    void drawLineCurve(NVGcontext *nvgContext, Curve *curve);
    void drawFillCurve(NVGcontext *nvgContext, Curve *curve);
    void drawCurveDescriptions(NVGcontext *nvgContext);
    void drawText(NVGcontext *nvgContext, float x, float y,
                  float fontSize,
                  const char *text, int color[4],
                  int halign, int valign);
    void drawSeparatorY0(NVGcontext *nvgContext);
    bool isCurveUndefined(const vector<float> &x,
                          const vector<float> &y,
                          int minNumValues);
    void setCurveDrawStyle(NVGcontext *nvgContext, Curve *curve);

 protected:
    vector<Curve *> _curves;
    Axis *_hAxis;
    Axis *_vAxis;
    
    int _width;
    int _height;
};

#endif
