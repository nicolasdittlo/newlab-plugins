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
                     float sampleRate, int bufferSize,
                     int width);

    virtual ~DenoiserSpectrum();

    void reset(int bufferSize, float sampleRate);

    void updateCurves(const vector<float> &signal,
                      const vector<float> &noise,
                      const vector<float> &noiseProfile,
                      bool isLearning);
        
 protected:
    void createAxes(SpectrumView *spectrumView,
                    float sampleRate, int bufferSize,
                    int width);

    void createCurves(float sampleRate);
        
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

#endif
