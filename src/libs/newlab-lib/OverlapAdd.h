#ifndef OVERLAP_ADD_H
#define OVERLAP_ADD_H

#include "CircularBuffer.h"
#include <juce_dsp/juce_dsp.h>

class OverlapAddProcessor
{
public:
    OverlapAddProcessor();
    virtual ~OverlapAddProcessor();
    
    virtual void ProcessFFT(vector<float> *compBuf, int chanNum);
    virtual void ProcessOutSamples(vector<float> *buff, int chanNum);
};

class OverlapAdd
{
public:
    OverlapAdd(int fftSize, int overlap, bool fft, bool ifft);
    virtual ~OverlapAdd();
    
    void AddProcessor(OverlapAddProcessor *processor);
    
    void Feed(const vector<float> &samples, int blockSize, int nChan);

    void GetOutSamples(vector<float> *samples);
    void ClearOutSamples();
    void FlushOutSamples(int numToFlush);
    
protected:
    virtual void ProcessFFT(vector<float> *compBuf, int chanNum);
    virtual void ProcessOutSamples(vector<float> *buff, int chanNum);

    // win must have been resized before
    virtual void MakeWindow(vector<float> *win);

    vector<OverlapAddProcessor *> _processors;
    
    int _fftSize;
    int _overlap;

    bool _fftFlag;
    bool _ifftFlag;
    
    CircularBuffer<float> _circSampBufsIn[2];
    CircularBuffer<float> _circSampBufsOut[2];
    
    vector<float> _tmpSampBufIn;
    vector<float> _tmpSampBufOut;
    vector<float> _tmpCompBufIn;
    vector<float> _tmpCompBufOut;

    vector<float> _tmpSynthZeroBuf;
    
    vector<float> _anaWin;
    vector<float> _synthWin;

    vector<float> _outSamples;

    juce::dsp::FFT _forwardFFT;
    juce::dsp::FFT _backwardFFT;
};

#endif // OVERLAP_ADD_H
