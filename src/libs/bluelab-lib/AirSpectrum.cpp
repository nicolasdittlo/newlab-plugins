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

#include "AirSpectrum.h"

#define CURVE_SMOOTH_COEFF_MS 1.4

#define AIR_MIN_DB -119.0
#define AIR_MAX_DB 10.0

#define CURVE_NUM_VALUES 256

AirSpectrum::AirSpectrum(SpectrumView *spectrumView,
                         float sampleRate, int bufferSize)
{
    _spectrumView = spectrumView;
    
    createAxes(spectrumView, sampleRate, bufferSize);

    createCurves(sampleRate);
}

AirSpectrum::~AirSpectrum()
{
    delete _freqAxis;
    delete _ampAxis;

    delete _hAxis;
    delete _vAxis;
    
    delete _airCurve;
    delete _airCurveSmooth;
    
    delete _harmoCurve;
    delete _harmoCurveSmooth;
    
    delete _sumCurve;
    delete _sumCurveSmooth;
}

void
AirSpectrum::createAxes(SpectrumView *spectrumView,
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
    
    _ampAxis->init(_vAxis, AIR_MIN_DB, AIR_MAX_DB);
}

void
AirSpectrum::createCurves(float sampleRate)
{
#define REF_SAMPLERATE 44100.0
    float curveSmoothCoeff =
        ParamSmoother::computeSmoothFactor(CURVE_SMOOTH_COEFF_MS, REF_SAMPLERATE);

    int descrColor[4] = { 170, 170, 170, 255 };    
    float fillAlpha = 0.5;
    
    // Air curve
    int airColor[4] = { 49, 188, 255, 255 };

    _airCurve = new Curve(CURVE_NUM_VALUES);
    _airCurve->setDescription("air", descrColor);
    _airCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _airCurve->setYScale(Scale::DB, AIR_MIN_DB, AIR_MAX_DB);
    _airCurve->setColor(airColor[0], airColor[1], airColor[2]);
    _airCurve->setFill(true);
    _airCurve->setFillColor(airColor[0], airColor[1], airColor[2], fillAlpha*255);
    _airCurve->setLineWidth(2.0);
        
    _airCurveSmooth = new SmoothCurveDB(_airCurve,
                                        curveSmoothCoeff,
                                        CURVE_NUM_VALUES,
                                        AIR_MIN_DB,
                                        AIR_MIN_DB, AIR_MAX_DB,
                                        sampleRate);
	
    // Harmo curve
    int harmoColor[4] = { 162, 61, 243, 255 };
    
    _harmoCurve = new Curve(CURVE_NUM_VALUES);
    _harmoCurve->setDescription("harmo", descrColor);
    _harmoCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _harmoCurve->setYScale(Scale::DB, AIR_MIN_DB, AIR_MAX_DB);
    _harmoCurve->setColor(harmoColor[0], harmoColor[1], harmoColor[2]);
    _harmoCurve->setFill(true);
    _harmoCurve->setFillColor(harmoColor[0], harmoColor[1], harmoColor[2], fillAlpha*255);
    
    _harmoCurveSmooth = new SmoothCurveDB(_harmoCurve,
                                          curveSmoothCoeff,
                                          CURVE_NUM_VALUES,
                                          AIR_MIN_DB,
                                          AIR_MIN_DB, AIR_MAX_DB,
                                          sampleRate);
        
    // Sum curve
    int sumColor[4] = { 200, 200, 255, 255 };
        
    _sumCurve = new Curve(CURVE_NUM_VALUES);
    _sumCurve->setDescription("noise profile", descrColor);
    _sumCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _sumCurve->setYScale(Scale::DB, AIR_MIN_DB, AIR_MAX_DB);
    _sumCurve->setColor(sumColor[0], sumColor[1], sumColor[2]);
        
    float curveSmoothCoeffNoiseProfile =
        ParamSmoother::computeSmoothFactor(CURVE_SMOOTH_COEFF_MS,
                                           REF_SAMPLERATE);
    _sumCurveSmooth = new SmoothCurveDB(_sumCurve,
                                        curveSmoothCoeffNoiseProfile,
                                        CURVE_NUM_VALUES,
                                        AIR_MIN_DB,
                                        AIR_MIN_DB,
                                        AIR_MAX_DB,
                                        sampleRate);

    int size[2];
    _spectrumView->getViewSize(&size[0], &size[1]);
    
    _airCurve->setViewSize(size[0], size[1]);
    _spectrumView->addCurve(_airCurve);
    
    _harmoCurve->setViewSize(size[0], size[1]);
    _spectrumView->addCurve(_harmoCurve);
    
    _sumCurve->setViewSize(size[0], size[1]);
    _spectrumView->addCurve(_sumCurve);
}

void
AirSpectrum::reset(int bufferSize, float sampleRate)
{
    _freqAxis->reset(bufferSize, sampleRate);

    _airCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _harmoCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _sumCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);

    float curveSmoothCoeff =
        ParamSmoother::computeSmoothFactor(CURVE_SMOOTH_COEFF_MS, sampleRate);
        
    _airCurveSmooth->reset(sampleRate, curveSmoothCoeff);
    _harmoCurveSmooth->reset(sampleRate, curveSmoothCoeff);
    _sumCurveSmooth->reset(sampleRate, curveSmoothCoeff);
}

void
AirSpectrum::updateCurves(const vector<float> &airCurve,
                          const vector<float> &harmoCurve,
                          const vector<float> &sumCurve)
{
    _airCurveSmooth->setValues(airCurve);

    _harmoCurveSmooth->setValues(harmoCurve);

    _sumCurveSmooth->setValues(sumCurve);
}

void
AirSpectrum::setMix(float mix)
{
    float fillAlpha = 0.5f;
    
    if (mix <= 0.0f)
    {
        float t = 1.0f + mix;
        
        t = pow(t, 1.0f/2.0f);
        
        _harmoCurve->setAlpha(t);
        _harmoCurve->setFillAlpha(t*fillAlpha);
    
        _airCurve->setAlpha(1.0f);
        _airCurve->setFillAlpha(fillAlpha);
    }
    
    if (mix >= 0.0f)
    {
        float t = (1.0f - mix);

        t = pow(t, 1.0f/2.0f);
        
        _airCurve->setAlpha(t);
        _airCurve->setFillAlpha(t*fillAlpha);
        
        _harmoCurve->setAlpha(1.0f);
        _harmoCurve->setFillAlpha(fillAlpha);
    }
}
