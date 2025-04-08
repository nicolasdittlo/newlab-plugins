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

#ifndef SPECTRUM_VIEW_JUCE_H
#define SPECTRUM_VIEW_JUCE_H

#include <vector>
using namespace std;

#include "SpectrumView.h"

class SpectrumViewJuce : public SpectrumView
{
 public:
    SpectrumViewJuce();
    virtual ~SpectrumViewJuce();
    
    void paint(juce::Graphics &g);
    
 protected:
    void drawAxis(juce::Graphics &g, bool lineLabelFlag);
    void drawAxis(juce::Graphics &g, Axis *axis, bool horizontal, bool lineLabelFlag);
    void drawCurves(juce::Graphics &g);
    void drawLineCurve(juce::Graphics &g, Curve *curve);
    void drawFillCurve(juce::Graphics &g, Curve *curve);
    void drawCurveDescriptions(juce::Graphics &g);
    void drawText(juce::Graphics &g, float x, float y,
                  float fontSize,
                  const char *text, int color[4],
                  juce::Justification align);
    void drawSeparatorY0(juce::Graphics &g);
    bool isCurveUndefined(const vector<float> &x,
                          const vector<float> &y,
                          int minNumValues);

 protected:
    juce::Font _font;
};

#endif
