#include <math.h>
#include <juce_dsp/juce_dsp.h>

#include "Window.h"
#include "OverlapAdd.h"

#ifndef M_PI
#define M_PI 3.141592654
#endif

// OverlapAddProcessor
OverlapAddProcessor::OverlapAddProcessor() {}

OverlapAddProcessor::~OverlapAddProcessor() {}

void
OverlapAddProcessor::processFFT(vector<complex> *compBuf) {}

void
OverlapAddProcessor::processOutSamples(vector<float> *buff) {}

// OverlapAdd
OverlapAdd::OverlapAdd(int fftSize, int overlap, bool fft, bool ifft)
: _forwardFFT(log2(fftSize)), _backwardFFT(log2(fftSize))
{
    _overlap = overlap;

    _fftFlag = fft;
    _ifftFlag = ifft;
    
    setFftSize(fftSize);
}

OverlapAdd::~OverlapAdd() {}

void
OverlapAdd::setFftSize(int fftSize)
{
    _forwardFFT = juce::FFT(log2(fftSize));
    _backwardFFT = juce::FFT(log2(fftSize));

    _fftSize = fftSize;

    vector<float> zeros;
    zeros.resize(_fftSize * 2);
    memset(zeros.data(), 0, zeros.size() * sizeof(float));

    for (int i = 0; i < 2; i++)
    {
        _circSampBufsIn[i].SetCapacity(_fftSize * 2);
        _circSampBufsOut[i].SetCapacity(_fftSize * 2);

        _circSampBufsOut[i].Push(zeros.data(), zeros.size());
    }

    _tmpSampBufIn.resize(_fftSize);
    _tmpSampBufOut.resize(_fftSize);
    _tmpCompBufOut.resize(_fftSize/2 + 1);

    _anaWin.resize(_fftSize);
    Window::makeWindowHan(&_anaWin);

    _synthWin.resize(_fftSize);
    Window::makeWindowHan(&_synthWin);
}

void
OverlapAdd::setOverlap(int overlap)
{
    _overlap = overlap;

    vector<float> zeros;
    zeros.resize(_fftSize * 2);
    memset(zeros.data(), 0, zeros.size() * sizeof(float));
    
    for (int i = 0; i < 2; i++)
    {
        _circSampBufsIn[i].SetCapacity(_fftSize * 2);
        _circSampBufsOut[i].SetCapacity(_fftSize * 2);
        
        _circSampBufsOut[i].Push(zeros.data(), zeros.size());
    }
}

void
OverlapAdd::addProcessor(OverlapAddProcessor *processor)
{
    _processors.push_back(processor);
}

void
OverlapAdd::feed(const vector<float> &samples)
{
    int synthShift = _fftSize / _overlap;
    _tmpSynthZeroBuf.resize(synthShift);
    memset(_tmpSynthZeroBuf.data(), 0, _tmpSynthZeroBuf.size() * sizeof(float));

    for (int i = 0; i < nChan; i++)
        _circSampBufsIn[i].Push(samples.data(), samples.size());

    while (_circSampBufsIn.GetSize() > _fftSize)
    {
        // Get current buffer
        _circSampBufsIn.Peek(_tmpSampBufIn.data(), _fftSize);
        _circSampBufsIn.Pop(_fftSize / _overlap);

        if (_fftFlag)
        {
            // Apply analysis window
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
                _tmpSampBufIn[k] *= _anaWin[k];
            
            // Convert real input to JUCE format
            juce::HeapBlock<float> fftInput(_fftSize);
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
                fftInput[k] = _tmpSampBufIn[k];

            // Output buffer
            juce::HeapBlock<float> fftOutput(_fftSize * 2);

            // Apply FFT
            _forwardFFT.performRealOnlyForwardTransform(fftInput.get());

            // Store output in temporary buffer
            for (int k = 0; k < _tmpCompBufOut.size(); k++)
                _tmpCompBufOut[k] = complex(fftInput[k], fftInput[_fftSize + k]);
        }

        // Apply callback
        processFFT(&_tmpCompBufOut);

        if (_ifftFlag)
        {
            // Convert to JUCE real format for inverse FFT
            juce::HeapBlock<float> ifftInput(_fftSize * 2);
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
            {
                ifftInput[k] = _tmpCompBufOut[k].real();
                ifftInput[_fftSize + k] = _tmpCompBufOut[k].imag();
            }
            
            // Apply inverse FFT
            _backwardFFT.performRealOnlyInverseTransform(ifftInput.get());

            // Convert back to real samples
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
                _tmpSampBufIn[k] = ifftInput[k];
            
            // Apply resynth coeff
            double resynthCoeff = 1.0 / _fftSize;
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

            _circSampBufsOut.Poke(_tmpSampBufIn.data(),
                                     _synthWin.size());

            _circSampBufsOut.Pop(synthShift);

            _circSampBufsOut.Push(_tmpSynthZeroBuf.data(),
                                  _tmpSynthZeroBuf.size());

            // Apply callback
            ProcessOutSamples(&_tmpSampBufIn);
        }
    }
}

void
OverlapAdd::getOutSamples(vector<float> *samples)
{
    *samples = _outSamples;
}

void
OverlapAdd::clearOutSamples()
{
    _outSamples.clear();
}

void
OverlapAdd::flushOutSamples(int numToFlush)
{
    if (numToFlush > _outSamples.size())
    {
        _outSamples.clear();
        return;
    }

    _outSamples.erase(_outSamples.begin(), _outSamples.begin() + numToFlush);
}

void
OverlapAdd::processFFT(vector<float> *compBuf)
{
    for (int i = 0; i < _processors.size(); i++)
    {
        OverlapAddProcessor *processor = _processors[i];
        processor->ProcessFFT(compBuf);
    }
}

void
OverlapAdd::processOutSamples(vector<float> *buff)
{
    for (int i = 0; i < _processors.size(); i++)
    {
        OverlapAddProcessor *processor = _processors[i];
        processor->ProcessOutSamples(buff);
    }

    int size = _outSamples.size();
    _outSamples.resize(size + _fftSize / _overlap);
    memcpy(&_outSamples.data()[size], buff->data(), (_fftSize / _overlap) * sizeof(float));
}
