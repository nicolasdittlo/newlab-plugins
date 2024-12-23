#ifndef DENOISER_PROCESSOR_H
#define DENOISER_PROCESSOR_H

#include <nl_queue.h>
#include <OverlapAdd.h>

// Definitions
#define USE_AUTO_RES_NOISE 1

#define DENOISER_MIN_DB -119.0
#define DENOISER_MAX_DB 10.0

class WienerSoftMasking;
class DenoiserProcessor : public OverlapAddProcessor
{
public:
    DenoiserProcessor(int bufferSize, int oversampling, float threshold);
    
    virtual ~DenoiserProcessor();
    
    void reset(int bufferSize, int oversampling, float sampleRate);
    
    void setThreshold(float threshold);
    
    void processFft(vector<complex> *ioBuffer) override;
    
    void getSignalBuffer(vector<float> *ioBuffer);
    
    void getNoiseBuffer(vector<float> *ioBuffer);
    
    // Noise capture
    void setBuildingNoiseStatistics(bool flag);
    
    void addNoiseStatistics(const vector<complex> &buf);
    
    //
    void getNoiseCurve(vector<float> *noiseCurve);
    void setNoiseCurve(const vector<float> &noiseCurve);
    
    // Used for serialization
    void getNativeNoiseCurve(vector<float> *noiseCurve);
    void setNativeNoiseCurve(const vector<float> &noiseCurve);
    
    void setResNoiseThrs(float threshold);

#if USE_AUTO_RES_NOISE
    void setAutoResNoise(bool autoResNoiseFlag);
#endif
    
    static void applyThresholdToNoiseCurve(vector<float> *ioNoiseCurve, float threshold);

    int getLatency();
    
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
                             vector<float> *ioNoiseMagns,
                             vector<float> *ioSignalPhases,
                             vector<float> *ioNoisePhases);
#endif
    
    // Kernel can be NULL
    void noiseFilter(float *input, float *output, int width, int height,
                     int winSize, vector<float> *kernel, int lineNum,
                     float threshold);
    
    // Take an fft buffer history and transform it to an image
    void samplesHistoryToImage(const bl_queue<vector<float> > *hist,
                               vector<float> *imageChunk);
    
    // Take an image and extract one line
    // Fill an Fft buffer
    // Take the phases from the history
    void imageLineToSamples(const vector<float> *image,
                            int width, int height, int lineNum,
                            const bl_queue<vector<float> > *hist,
                            const bl_queue<vector<float> > *phaseHist,
                            vector<float> *resultBuf,
                            vector<float> *resultPhases);
    
    void extractResidualNoise(const vector<float> *prevSignal,
                              const vector<float> *signal,
                              vector<float> *ioNoise);
    
    // Soft or hard elbow
    void threshold(vector<float> *ioSigMagns, vector<float> *ioNoiseMagns);
    
    void resampleNoiseCurve();

    void MakeHanningKernel2D(int size, vector<float> *result);

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
    
    // Noise capture
    bool _isBuildingNoiseStatistics;
    
    vector<float> _noiseCurve;
    
    // Used to keep original noise curve if need to rescale
    vector<float> _nativeNoiseCurve;
    
    // Residual denoise
    
    // NOTE: since using bl_queue, we push_back instead() of push_front()
    bl_queue<vector<float> > _historyFftBufs;
    bl_queue<vector<float> > _historyFftNoiseBufs;
    bl_queue<vector<float> > _historyPhases;
    
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
    vector<complex> _tmpBuf7;
    
    vector<float> _tmpBuf9;
    vector<float> _tmpBuf10;
    vector<complex> _tmpBuf11;
    vector<complex> _tmpBuf12;
    
    vector<float> _tmpBuf16;
    vector<float> _tmpBuf17;

    vector<float> _tmpBuf19;
    vector<complex> _tmpBuf20;
    
    vector<complex> _tmpBuf22;
    vector<complex> _tmpBuf23;
    vector<float> _tmpBuf24;
};

#endif
