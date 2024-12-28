#ifndef DENOISER_PROCESSOR_H
#define DENOISER_PROCESSOR_H

#include "nl_queue.h"
#include "OverlapAdd.h"

#define USE_AUTO_RES_NOISE 1

class WienerSoftMasking;
class DenoiserProcessor : public OverlapAddProcessor
{
public:
    DenoiserProcessor(int bufferSize, int overlap, float threshold);
    
    virtual ~DenoiserProcessor();

    void processFFT(vector<complex<float> > *ioBuffer) override;
    
    void reset(int bufferSize, int overlap, float sampleRate);

    void setOverlap(int overlap);
    
    void setThreshold(float threshold);
    
    void getSignalBuffer(vector<float> *ioBuffer);
    
    void getNoiseBuffer(vector<float> *ioBuffer);
    
    // Noise capture
    void setBuildingNoiseStatistics(bool flag);
    
    void addNoiseStatistics(const vector<complex<float> > &buf);
    
    void getNoiseCurve(vector<float> *noiseCurve);
    void setNoiseCurve(const vector<float> &noiseCurve);
    
    // Used for serialization
    void getNativeNoiseCurve(vector<float> *noiseCurve);
    void setNativeNoiseCurve(const vector<float> &noiseCurve);
    
    void setResNoiseThrs(float threshold);

#if USE_AUTO_RES_NOISE
    void setAutoResNoise(bool autoResNoiseFlag);
#endif

    void setRatio(float ratio);

    void setNoiseOnly(bool noiseOnly);
    
    int getLatency();

    static void applyThresholdValueToNoiseCurve(vector<float> *ioNoiseCurve, float threshold);
    
protected:
    
    // Residual denoise
    
    void resetResNoiseHistory();
    
    // Must keep and manage the phases
    // (there is an history, and we must have synchronous phases)
    void residualDenoise(vector<float> *signalBuffer,
                         vector<float> *noiseBuffer,
                         vector<float> *phases);
    
#if USE_AUTO_RES_NOISE
    void autoResidualDenoise(vector<float> *ioSignalMagns,
                             vector<float> *ioSignalPhases,
                             const vector<float> &noiseMagns);
#endif
    
    // Kernel can be NULL
    void noiseFilter(float *input, float *output, int width, int height,
                     int winSize, vector<float> *kernel, int lineNum,
                     float threshold);
    
    // Take an fft buffer history and transform it to an image
    void samplesHistoryToImage(const nl_queue<vector<float> > *hist,
                               vector<float> *imageChunk);
    
    // Take an image and extract one line
    // Fill an Fft buffer
    // Take the phases from the history
    void imageLineToSamples(const vector<float> *image,
                            int width, int height, int lineNum,
                            const nl_queue<vector<float> > *hist,
                            const nl_queue<vector<float> > *phaseHist,
                            vector<float> *resultBuf,
                            vector<float> *resultPhases);
    
    void extractResidualNoise(const vector<float> *prevSignal,
                              const vector<float> *signal,
                              vector<float> *ioNoise);
    
    // Soft or hard elbow
    void threshold(vector<float> *ioSigMagns, vector<float> *ioNoiseMagns);
    
    void resampleNoiseCurve();

    void makeHanningKernel2D(int size, vector<float> *result);

    int _bufferSize;
    int _overlap;
    float _sampleRate;
    
    vector<float> _signalBuf;
    vector<float> _noiseBuf;
    
    float _threshold;
    
    // Residual noise
    float _resNoiseThrs;
    
#if USE_AUTO_RES_NOISE
    bool _autoResNoise;
    WienerSoftMasking *_softMasking;
#endif

    float _ratio;

    bool _noiseOnly;
    
    // Noise capture
    bool _isBuildingNoiseStatistics;
    
    vector<float> _noiseCurve;
    
    // Used to keep original noise curve if need to rescale
    vector<float> _nativeNoiseCurve;
    
    // Residual denoise
    
    nl_queue<vector<float> > _historyFftBufs;
    nl_queue<vector<float> > _historyFftNoiseBufs;
    nl_queue<vector<float> > _historyPhases;
    
    vector<float> _inputImageFilterChunk;
    vector<float> _outputImageFilterChunk;
    
    vector<float> _hanningKernel;

private:
    // Tmp buffers
    vector<float> _tmpBuf0;
    vector<float> _tmpBuf1;
    vector<float> _tmpBuf2;
    vector<float> _tmpBuf3;
    vector<float> _tmpBuf4;
    vector<float> _tmpBuf5;
    vector<float> _tmpBuf6;
    vector<complex<float> > _tmpBuf7;
    
    vector<float> _tmpBuf9;
    vector<float> _tmpBuf10;
    vector<complex<float> > _tmpBuf11;
    vector<complex<float> > _tmpBuf12;
    
    vector<float> _tmpBuf16;
    vector<float> _tmpBuf17;

    vector<float> _tmpBuf19;
    vector<complex<float> > _tmpBuf20;
    
    vector<complex<float> > _tmpBuf22;
    vector<complex<float> > _tmpBuf23;
    vector<float> _tmpBuf24;
};

#endif
