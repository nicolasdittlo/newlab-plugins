#include "Axis.h"
#include "FreqAxis.h"
#include "AmpAxis.h"
#include "Curve.h"
#include "SmoothCurveDB.h"
#include "ParamSmoother.h"
#include "SpectrumView.h"

#include "DenoiserSpectrum.h"

#define CURVE_SMOOTH_COEFF_MS 1.4
#define CURVE_SMOOTH_COEFF_NOISE_PROFILE_MS 0.5

#define DENOISER_MIN_DB -119.0
#define DENOISER_MAX_DB 10.0

#define CURVE_NUM_VALUES 256

DenoiserSpectrum::DenoiserSpectrum(SpectrumView *spectrumView,
                                   float sampleRate, int bufferSize,
                                   int width)
{
    _spectrumView = spectrumView;
    
    createAxes(spectrumView, sampleRate, bufferSize, width);

    createCurves(sampleRate);
}

DenoiserSpectrum::~DenoiserSpectrum()
{
    delete _freqAxis;
    delete _ampAxis;

    delete _hAxis;
    delete _vAxis;
    
    delete _signalCurve;
    delete _signalCurveSmooth;
    
    delete _noiseCurve;
    delete _noiseCurveSmooth;
    
    delete _noiseProfileCurve;
    delete _noiseProfileCurveSmooth;
}

void
DenoiserSpectrum::createAxes(SpectrumView *spectrumView,
                             float sampleRate, int bufferSize,
                             int width)
{
    _hAxis = new Axis();
    _freqAxis = new FreqAxis(true, Scale::LOG);
    
    _vAxis = new Axis();
    _ampAxis = new AmpAxis();
    
    spectrumView->setHAxis(_hAxis);
    spectrumView->setVAxis(_vAxis);
    
    bool horizontal = true;
    _freqAxis->init(_hAxis, horizontal, bufferSize,
                    sampleRate, width);
    _freqAxis->reset(bufferSize, sampleRate);
    
    _ampAxis->init(_vAxis, DENOISER_MIN_DB, DENOISER_MAX_DB, width);
}

void
DenoiserSpectrum::createCurves(float sampleRate)
{
#define REF_SAMPLERATE 44100.0
    float curveSmoothCoeff =
        ParamSmoother::computeSmoothFactor(CURVE_SMOOTH_COEFF_MS, REF_SAMPLERATE);

    // Signal curve
    int descrColor[4] = { 170, 170, 170, 255 };    
    float fillAlpha = 0.5;
    int signalColor[4] = { 64, 64, 255, 255 };

    _signalCurve = new Curve(CURVE_NUM_VALUES);
    _signalCurve->setDescription("signal", descrColor);
    _signalCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _signalCurve->setYScale(Scale::DB, DENOISER_MIN_DB, DENOISER_MAX_DB);
    _signalCurve->setFill(true);
    _signalCurve->setFillAlpha(fillAlpha);
    _signalCurve->setColor(signalColor[0], signalColor[1], signalColor[2]);
    _signalCurve->setLineWidth(2.0);
        
    _signalCurveSmooth = new SmoothCurveDB(_signalCurve,
                                           curveSmoothCoeff,
                                           CURVE_NUM_VALUES,
                                           DENOISER_MIN_DB,
                                           DENOISER_MIN_DB, DENOISER_MAX_DB,
                                           sampleRate);
	
    // Noise curve
    int noiseColor[4] = { 255, 255, 200, 200 };
    
    _noiseCurve = new Curve(CURVE_NUM_VALUES);
    _noiseCurve->setDescription("noise", descrColor);
    _noiseCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _noiseCurve->setYScale(Scale::DB, DENOISER_MIN_DB, DENOISER_MAX_DB);
    _noiseCurve->setFillAlpha(fillAlpha);
    _noiseCurve->setColor(noiseColor[0], noiseColor[1], noiseColor[2]);
        
    _noiseCurveSmooth = new SmoothCurveDB(_noiseCurve,
                                          curveSmoothCoeff,
                                          CURVE_NUM_VALUES,
                                          DENOISER_MIN_DB,
                                          DENOISER_MIN_DB, DENOISER_MAX_DB,
                                          sampleRate);
        
    // Noise profile curve
    int noiseProfileColor[4] = { 255, 128, 0, 255 };
        
    _noiseProfileCurve = new Curve(CURVE_NUM_VALUES);
    _noiseProfileCurve->setDescription("noise profile", descrColor);
    _noiseProfileCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _noiseProfileCurve->setYScale(Scale::DB, DENOISER_MIN_DB, DENOISER_MAX_DB);
    _noiseProfileCurve->setFillAlpha(fillAlpha);
    _noiseProfileCurve->setColor(noiseProfileColor[0],
                                 noiseProfileColor[1],
                                 noiseProfileColor[2]);
        
    float curveSmoothCoeffNoiseProfile =
        ParamSmoother::computeSmoothFactor(CURVE_SMOOTH_COEFF_NOISE_PROFILE_MS,
                                           REF_SAMPLERATE);
    _noiseProfileCurveSmooth = new SmoothCurveDB(_noiseProfileCurve,
                                                 curveSmoothCoeffNoiseProfile,
                                                 CURVE_NUM_VALUES,
                                                 DENOISER_MIN_DB,
                                                 DENOISER_MIN_DB,
                                                 DENOISER_MAX_DB,
                                                 sampleRate);

    int size[2];
    _spectrumView->getViewSize(&size[0], &size[1]);
    
    _signalCurve->setViewSize(size[0], size[1]);
    _spectrumView->addCurve(_signalCurve);
    
    _noiseCurve->setViewSize(size[0], size[1]);
    _spectrumView->addCurve(_noiseCurve);
    
    _noiseProfileCurve->setViewSize(size[0], size[1]);
    _spectrumView->addCurve(_noiseProfileCurve);
}

void
DenoiserSpectrum::reset(int bufferSize, float sampleRate)
{
    _freqAxis->reset(bufferSize, sampleRate);

    _signalCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _noiseCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);
    _noiseProfileCurve->setXScale(Scale::LOG, 0.0, sampleRate*0.5);


    float curveSmoothCoeff =
        ParamSmoother::computeSmoothFactor(CURVE_SMOOTH_COEFF_MS, sampleRate);
        
    _signalCurveSmooth->reset(sampleRate, curveSmoothCoeff);
    _noiseCurveSmooth->reset(sampleRate, curveSmoothCoeff);
    _noiseProfileCurveSmooth->reset(sampleRate, curveSmoothCoeff);
}

void
DenoiserSpectrum::updateCurves(const vector<float> &signal,
                               const vector<float> &noise,
                               const vector<float> &noiseProfile,
                               bool isLearning)
{
    _signalCurveSmooth->setValues(signal, false);

    if (!isLearning)
        _noiseCurveSmooth->setValues(noise, false);
    else
        _noiseCurveSmooth->clearValues();

    _noiseProfileCurveSmooth->setValues(noiseProfile, false);
}
