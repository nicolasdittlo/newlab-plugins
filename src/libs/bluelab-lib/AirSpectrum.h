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

#ifndef AIR_SPECTRUM_H
#define AIR_SPECTRUM_H

#include <vector>
using namespace std;

class SpectrumViewNVG;
class Axis;
class AmpAxis;
class FreqAxis;
class Curve;
class SmoothCurveDB;

class AirSpectrum
{
 public:
    AirSpectrum(SpectrumViewNVG *spectrumView,
                float sampleRate, int bufferSize);

    virtual ~AirSpectrum();

    void reset(int bufferSize, float sampleRate);

    void updateCurves(const vector<float> &airCurve,
                      const vector<float> &harmoCurve,
                      const vector<float> &sumCurve);

    void setMix(float mix);
    
 protected:
    void createAxes(SpectrumViewNVG *spectrumView,
                    float sampleRate, int bufferSize);

    void createCurves(float sampleRate);

    SpectrumViewNVG *_spectrumView;
    
    AmpAxis *_ampAxis;
    Axis *_hAxis;
    
    FreqAxis *_freqAxis;
    Axis *_vAxis;

    Curve *_airCurve;
    SmoothCurveDB *_airCurveSmooth;
    
    Curve *_harmoCurve;
    SmoothCurveDB *_harmoCurveSmooth;
    
    Curve *_sumCurve;
    SmoothCurveDB *_sumCurveSmooth;
};

#endif
