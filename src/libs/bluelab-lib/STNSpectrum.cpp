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

#include "Axis.h"
#include "FreqAxis.h"
#include "AmpAxis.h"
#include "Curve.h"
#include "SmoothCurveDB.h"
#include "ParamSmoother.h"
#include "SpectrumView.h"

#include "STNSpectrum.h"

#define CURVE_SMOOTH_COEFF_MS 1.4

#define STN_MIN_DB -119.0
#define STN_MAX_DB 10.0

#define CURVE_NUM_VALUES 256

STNSpectrum::STNSpectrum(SpectrumView *spectrumView,
                         float sampleRate, int bufferSize)
{
    _spectrumView = spectrumView;
    
    createAxes(spectrumView, sampleRate, bufferSize);

    createCurves(sampleRate);
}

STNSpectrum::~STNSpectrum()
{
    delete _freqAxis;
    delete _ampAxis;

    delete _hAxis;
    delete _vAxis;
    
    delete _noiseCurve;
    delete _noiseCurveSmooth;
    
    delete _sinesCurve;
    delete _sinesCurveSmooth;
    
    delete _sumCurve;
    delete _sumCurveSmooth;
}

void
STNSpectrum::createAxes(SpectrumView *spectrumView,
                        float sampleRate, int bufferSize)
{
    _hAxis = new Axis();
    _freqAxis = new FreqAxis(true, Scale::LOG);
    
    _vAxis = new Axis();
    _ampAxis = new AmpAxis();
    
    spectrumView->setHAxis(_hAxis);
    spectrumView->setVAxis(_vAxis);
    
    _freqAxis->init(_hAxis, bufferSize, sampleRate);
    _freqAxis->reset(bufferSize, sampleRate);
    
    _ampAxis->init(_vAxis, STN_MIN_DB, STN_MAX_DB);
}

void
STNSpectrum::createCurves(float sampleRate)
{
#define REF_SAMPLERATE 44100.0
    float curveSmoothCoeff =
        ParamSmoother::computeSmoothFactor(CURVE_SMOOTH_COEFF_MS, REF_SAMPLERATE);

    int descrColor[4] = { 170, 170, 170, 255 };    
    float fillAlpha = 0.5;
    
    // STN curve
    int noiseColor[4] = { 128, 128, 128, 255 };

    _noiseCurve = new Curve(CURVE_NUM_VALUES);
    _noiseCurve->setDescription("noise", descrColor);
    _noiseCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _noiseCurve->setYScale(Scale::DB, STN_MIN_DB, STN_MAX_DB);
    _noiseCurve->setColor(noiseColor[0], noiseColor[1], noiseColor[2]);
    _noiseCurve->setFill(true);
    _noiseCurve->setFillColor(noiseColor[0], noiseColor[1], noiseColor[2], fillAlpha*255);
    _noiseCurve->setLineWidth(2.0);
        
    _noiseCurveSmooth = new SmoothCurveDB(_noiseCurve,
                                          curveSmoothCoeff,
                                          CURVE_NUM_VALUES,
                                          STN_MIN_DB,
                                          STN_MIN_DB, STN_MAX_DB,
                                          sampleRate);
	
    // Sines curve
    int sinesColor[4] = { 255, 0, 255, 255 };
    
    _sinesCurve = new Curve(CURVE_NUM_VALUES);
    _sinesCurve->setDescription("sines", descrColor);
    _sinesCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _sinesCurve->setYScale(Scale::DB, STN_MIN_DB, STN_MAX_DB);
    _sinesCurve->setColor(sinesColor[0], sinesColor[1], sinesColor[2]);
    _sinesCurve->setFill(true);
    _sinesCurve->setFillColor(sinesColor[0], sinesColor[1], sinesColor[2], fillAlpha*255);
    
    _sinesCurveSmooth = new SmoothCurveDB(_sinesCurve,
                                          curveSmoothCoeff,
                                          CURVE_NUM_VALUES,
                                          STN_MIN_DB,
                                          STN_MIN_DB, STN_MAX_DB,
                                          sampleRate);
        
    // Sum curve
    int sumColor[4] = { 200, 200, 255, 255 };
        
    _sumCurve = new Curve(CURVE_NUM_VALUES);
    _sumCurve->setDescription("out", descrColor);
    _sumCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _sumCurve->setYScale(Scale::DB, STN_MIN_DB, STN_MAX_DB);
    _sumCurve->setColor(sumColor[0], sumColor[1], sumColor[2]);
        
    _sumCurveSmooth = new SmoothCurveDB(_sumCurve,
                                        curveSmoothCoeff,
                                        CURVE_NUM_VALUES,
                                        STN_MIN_DB,
                                        STN_MIN_DB,
                                        STN_MAX_DB,
                                        sampleRate);

    int size[2];
    _spectrumView->getViewSize(&size[0], &size[1]);
    
    _noiseCurve->setViewSize(size[0], size[1]);
    _spectrumView->addCurve(_noiseCurve);
    
    _sinesCurve->setViewSize(size[0], size[1]);
    _spectrumView->addCurve(_sinesCurve);
    
    _sumCurve->setViewSize(size[0], size[1]);
    _spectrumView->addCurve(_sumCurve);
}

void
STNSpectrum::reset(int bufferSize, float sampleRate)
{
    _freqAxis->reset(bufferSize, sampleRate);

    _noiseCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _sinesCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _sumCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);

    float curveSmoothCoeff =
        ParamSmoother::computeSmoothFactor(CURVE_SMOOTH_COEFF_MS, sampleRate);
        
    _noiseCurveSmooth->reset(sampleRate, curveSmoothCoeff);
    _sinesCurveSmooth->reset(sampleRate, curveSmoothCoeff);
    _sumCurveSmooth->reset(sampleRate, curveSmoothCoeff);
}

void
STNSpectrum::updateCurves(const vector<float> &noiseCurve,
                          const vector<float> &sinesCurve,
                          const vector<float> &sumCurve)
{
    _noiseCurveSmooth->setValues(noiseCurve);

    _sinesCurveSmooth->setValues(sinesCurve);

    _sumCurveSmooth->setValues(sumCurve);
}
