#ifndef OVERLAP_ADD_H
#define OVERLAP_ADD_H

#include "CircularBuffer.h"
#include <juce_dsp/juce_dsp.h>

class OverlapAddProcessor
{
public:
    OverlapAddProcessor();
    virtual ~OverlapAddProcessor();
    
    virtual void processFFT(vector<complex> *compBuf);
    virtual void processOutSamples(vector<float> *buff);
};

class OverlapAdd
{
public:
    OverlapAdd(int fftSize, int overlap, bool fft, bool ifft);
    virtual ~OverlapAdd();

    void setFftSize(int fftSize);
    
    void addProcessor(OverlapAddProcessor *processor);
    
    void feed(const vector<float> &samples, int blockSize, int nChan);

    void getOutSamples(vector<float> *samples);
    void clearOutSamples();
    void flushOutSamples(int numToFlush);
    
protected:
    virtual void processFFT(vector<complex> *compBuf);
    virtual void processOutSamples(vector<float> *buff);

    vector<OverlapAddProcessor *> _processors;
    
    int _fftSize;
    int _overlap;

    bool _fftFlag;
    bool _ifftFlag;
    
    CircularBuffer<float> _circSampBufsIn[2];
    CircularBuffer<float> _circSampBufsOut[2];
    
    vector<complex> _tmpSampBufIn;
    vector<complex> _tmpSampBufOut;
    vector<complex> _tmpCompBufOut;

    vector<float> _tmpSynthZeroBuf;
    
    vector<float> _anaWin;
    vector<float> _synthWin;

    vector<float> _outSamples;

    juce::dsp::FFT _forwardFFT;
    juce::dsp::FFT _backwardFFT;
};

#endif // OVERLAP_ADD_H
