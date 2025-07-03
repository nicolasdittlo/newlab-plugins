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

#ifndef DENOISER_SPECTRUM_H
#define DENOISER_SPECTRUM_H

#include <vector>
using namespace std;

class SpectrumView;
class Axis;
class AmpAxis;
class FreqAxis;
class Curve;
class SmoothCurveDB;

class DenoiserSpectrum
{
 public:
    DenoiserSpectrum(SpectrumView *spectrumView,
                     float sampleRate, int bufferSize);

    virtual ~DenoiserSpectrum();

    void reset(int bufferSize, float sampleRate);

    void updateCurves(const vector<float> &signal,
                      const vector<float> &noise,
                      const vector<float> &noiseProfile,
                      bool isLearning);
        
 protected:
    void createAxes(SpectrumView *spectrumView,
                    float sampleRate, int bufferSize);

    void createCurves(float sampleRate);

    SpectrumView *_spectrumView;
    
    AmpAxis *_ampAxis;
    Axis *_hAxis;
    
    FreqAxis *_freqAxis;
    Axis *_vAxis;

    Curve *_signalCurve;
    SmoothCurveDB *_signalCurveSmooth;
    
    Curve *_noiseCurve;
    SmoothCurveDB *_noiseCurveSmooth;
    
    Curve *_noiseProfileCurve;
    SmoothCurveDB *_noiseProfileCurveSmooth;
};

#endif
