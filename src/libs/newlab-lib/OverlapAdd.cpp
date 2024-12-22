#include <math.h>

#include "OverlapAdd.h"

#ifndef M_PI
#define M_PI 3.141592654
#endif

// OverlapAddProcessor
OverlapAddProcessor::OverlapAddProcessor() {}

OverlapAddProcessor::~OverlapAddProcessor() {}
    
void
OverlapAddProcessor::ProcessFFT(vector<float> *compBuf, int chanNum) {}

void
OverlapAddProcessor::ProcessOutSamples(vector<float> *buff, int chanNum) {}

// OverlapAdd
OverlapAdd::OverlapAdd(int fftSize, int overlap, bool fft, bool ifft)
{
    _fftSize = fftSize;
    _overlap = overlap;

    _fftFlag = fft;
    _ifftFlag = ifft;
    
    vector<float> zeros;
    zeros.resize(_fftSize*2);
    memset(zeros.data(), 0, zeros.size()*sizeof(float));

    for (int i = 0; i < 2; i++)
    {
        _circSampBufsIn[i].SetCapacity(_fftSize*2);
        _circSampBufsOut[i].SetCapacity(_fftSize*2);
        
        _circSampBufsOut[i].Push(zeros.data(), zeros.size());
    }
    
    _tmpSampBufIn.resize(_fftSize);
    _tmpSampBufOut.resize(_fftSize);
    _tmpCompBufIn.resize(_fftSize*2);
    _tmpCompBufOut.resize(_fftSize*2);

    _anaWin.resize(_fftSize);
    MakeWindow(&_anaWin);

    _synthWin.resize(_fftSize);
    MakeWindow(&_synthWin);
    
    _forwardFFT = kiss_fft_alloc(_fftSize, 0, 0, 0);
    _backwardFFT = kiss_fft_alloc(_fftSize, 1, 0, 0);

    _tmpCompBufIn.resize(_fftSize*2);
    _tmpCompBufOut.resize(_fftSize*2);
}

OverlapAdd::~OverlapAdd()
{
    kiss_fft_free(_forwardFFT);
    kiss_fft_free(_backwardFFT);
}

void
OverlapAdd::AddProcessor(OverlapAddProcessor *processor)
{
    _processors.push_back(processor);
}

void
OverlapAdd::Feed(const vector<float> &samples, int blockSize, int nChan)
{    
    int synthShift = _fftSize/_overlap;
    _tmpSynthZeroBuf.resize(synthShift);
    memset(_tmpSynthZeroBuf.data(), 0, _tmpSynthZeroBuf.size()*sizeof(float));
    
    for (int i = 0; i < nChan; i++)
        _circSampBufsIn[i].Push(&samples.data()[blockSize*i], blockSize);
             
    while(_circSampBufsIn[0].GetSize() > _fftSize)
    {
        for (int i = 0; i < nChan; i++)
        {
            // Get current buffer
            _circSampBufsIn[i].Peek(_tmpSampBufIn.data(), _fftSize);
            _circSampBufsIn[i].Pop(_fftSize/_overlap);

            if (_fftFlag)
            {
                // Apply analysis window
                for (int k = 0; k < _tmpSampBufIn.size(); k++)
                    _tmpSampBufIn[k] *= _anaWin[k];
            
                // Make input complex buffer
                for (int k = 0; k < _tmpSampBufIn.size(); k++)
                {
                    _tmpCompBufIn[k*2] = _tmpSampBufIn[k];
                    _tmpCompBufIn[k*2 + 1] = 0.0;
                }
                
                // Apply FFT
                kiss_fft(_forwardFFT,
                         (const kiss_fft_cpx *)_tmpCompBufIn.data(),
                         (kiss_fft_cpx *)_tmpCompBufOut.data());
            }
            
            // Apply callback
            ProcessFFT(&_tmpCompBufOut, i);
                
            if (_ifftFlag)
            {
                // Apply inverse FFT
                kiss_fft(_backwardFFT,
                         (const kiss_fft_cpx *)_tmpCompBufOut.data(),
                         (kiss_fft_cpx *)_tmpCompBufIn.data());
                
                // Make output samples buffer
                for (int k = 0; k < _tmpSampBufIn.size(); k++)
                    _tmpSampBufIn[k] = _tmpCompBufIn[k*2];
                
                // Apply resynth coeff
                double resynthCoeff = 1.0/_fftSize;
                for (int k = 0; k < _tmpSampBufIn.size(); k++)
                    _tmpSampBufIn[k] *= resynthCoeff;
                                
                // Apply synthesis window
                for (int k = 0; k < _tmpSampBufIn.size(); k++)
                    _tmpSampBufIn[k] *= _synthWin[k];
                
                // Output
                _circSampBufsOut[i].Peek(_tmpSampBufOut.data(),
                                         _synthWin.size());
                
                for (int k = 0; k < _tmpSampBufIn.size(); k++)
                    _tmpSampBufIn[k] += _tmpSampBufOut[k];
                
                _circSampBufsOut[i].Poke(_tmpSampBufIn.data(),
                                         _synthWin.size());
                
                _circSampBufsOut[i].Pop(synthShift);
                
                _circSampBufsOut[i].Push(_tmpSynthZeroBuf.data(),
                                         _tmpSynthZeroBuf.size());
                
                // Apply callback
                ProcessOutSamples(&_tmpSampBufIn, i);
            }
        }
    }
}

void
OverlapAdd::GetOutSamples(vector<float> *samples)
{
    *samples = _outSamples;
}

void
OverlapAdd::ClearOutSamples()
{
    _outSamples.clear();
}

void
OverlapAdd::FlushOutSamples(int numToFlush)
{
    if (numToFlush > _outSamples.size())
    {
        _outSamples.clear();
        return;
    }

    _outSamples.erase(_outSamples.begin(), _outSamples.begin() + numToFlush);
}

void
OverlapAdd::ProcessFFT(vector<float> *compBuf, int chanNum)
{
    for (int i = 0; i < _processors.size(); i++)
    {
        OverlapAddProcessor *processor = _processors[i];
        processor->ProcessFFT(compBuf, chanNum);
    }
}

void
OverlapAdd::ProcessOutSamples(vector<float> *buff, int chanNum)
{
    for (int i = 0; i < _processors.size(); i++)
    {
        OverlapAddProcessor *processor = _processors[i];
        processor->ProcessOutSamples(buff, chanNum);
    }
    
    int size = _outSamples.size();
    _outSamples.resize(size + _fftSize/_overlap);
    memcpy(&_outSamples.data()[size], buff->data(), (_fftSize/_overlap)*sizeof(float));
}

void
OverlapAdd::MakeWindow(vector<float> *win)
{    
    // Hann
    for (int i = 0; i < win->size(); i++)
        (*win)[i] = 0.5*(1.0 - cos(2.0*M_PI*
                                   ((double)i)/(win->size() - 1)));
}
