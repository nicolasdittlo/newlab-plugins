#include <math.h>

#include <juce_dsp/juce_dsp.h>

#include "Window.h"
#include "OverlapAdd.h"

// OverlapAddProcessor
OverlapAddProcessor::OverlapAddProcessor() {}

OverlapAddProcessor::~OverlapAddProcessor() {}

void
OverlapAddProcessor::processFFT(vector<complex<float> > *compBuf) {}

void
OverlapAddProcessor::processOutSamples(vector<float> *buff) {}

// OverlapAdd
OverlapAdd::OverlapAdd(int fftSize, int overlap, bool fft, bool ifft)
: _overlap(overlap), _fftFlag(fft), _ifftFlag(ifft)
{
    setFftSize(fftSize);
}

OverlapAdd::~OverlapAdd() {}

void
OverlapAdd::setFftSize(int fftSize)
{
    _fftSize = fftSize;

    _forwardFFT = std::make_unique<juce::dsp::FFT>(log2(fftSize));
    _backwardFFT = std::make_unique<juce::dsp::FFT>(log2(fftSize));

    vector<float> zeros;
    zeros.resize(_fftSize * 2);
    memset(zeros.data(), 0, zeros.size() * sizeof(float));

    _circSampBufsIn.setCapacity(_fftSize * 2);
    _circSampBufsOut.setCapacity(_fftSize * 2);

    _circSampBufsOut.push(zeros.data(), zeros.size());

    _tmpSampBufIn.resize(_fftSize);
    _tmpSampBufOut.resize(_fftSize);
    _tmpCompBufOut.resize(_fftSize / 2 + 1);

    makeWindows();
}

void
OverlapAdd::setOverlap(int overlap)
{
    _overlap = overlap;

    vector<float> zeros;
    zeros.resize(_fftSize * 2);
    memset(zeros.data(), 0, zeros.size() * sizeof(float));

    _circSampBufsIn.setCapacity(_fftSize * 2);
    _circSampBufsOut.setCapacity(_fftSize * 2);

    _circSampBufsOut.push(zeros.data(), zeros.size());

    makeWindows();
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

    _circSampBufsIn.push(samples.data(), samples.size());

    while (_circSampBufsIn.getSize() >= _fftSize)
    {
        // Get current buffer
        _circSampBufsIn.peek(_tmpSampBufIn.data(), _fftSize);
        _circSampBufsIn.pop(_fftSize / _overlap);

        if (_fftFlag)
        {
            // Apply analysis window
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
                _tmpSampBufIn[k] *= _anaWin[k];
            
            // Convert real input to JUCE format
            juce::HeapBlock<float> fftInput(2*_fftSize);
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
                fftInput[k] = _tmpSampBufIn[k];
            
            // Apply FFT
            _forwardFFT->performRealOnlyForwardTransform(fftInput.get(), true);
            
            // Store output in temporary buffer
            for (int k = 0; k < _tmpCompBufOut.size(); k++)
                _tmpCompBufOut[k] = complex(fftInput[k*2], fftInput[k*2 + 1]);
        }
                    
        // Apply callback
        processFFT(&_tmpCompBufOut);
        
        if (_ifftFlag)
        {
            // Convert to JUCE real format for inverse FFT
            juce::HeapBlock<float> ifftInput(_fftSize * 2);
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
            {
                ifftInput[k*2] = _tmpCompBufOut[k].real();
                ifftInput[k*2 + 1] = _tmpCompBufOut[k].imag();
            }

            // Apply inverse FFT
            _backwardFFT->performRealOnlyInverseTransform(ifftInput.get());
            
            // Convert back to real samples
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
                _tmpSampBufIn[k] = ifftInput[k];

            // Apply resynth coeff
            float resynthCoeff = 1.0 / _fftSize;
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
                _tmpSampBufIn[k] *= resynthCoeff;

            // Apply synthesis window
            for (int k = 0; k < _tmpSampBufIn.size(); k++)
                _tmpSampBufIn[k] *= _synthWin[k];
            
            // Output
            _circSampBufsOut.peek(_tmpSampBufOut.data(),
                                  _synthWin.size());

            for (int k = 0; k < _tmpSampBufIn.size(); k++)
                _tmpSampBufIn[k] += _tmpSampBufOut[k];

            _circSampBufsOut.poke(_tmpSampBufIn.data(),
                                  _synthWin.size());

            _circSampBufsOut.pop(synthShift);

            _circSampBufsOut.push(_tmpSynthZeroBuf.data(),
                                  _tmpSynthZeroBuf.size());
            
            // Apply callback
            processOutSamples(&_tmpSampBufIn);
        }
    }
}

int
OverlapAdd::getOutSamples(vector<float> *samples, int numSamples)
{
    samples->resize(numSamples);
   
    int numZeros = numSamples - _outSamples.size();
    if (numZeros < 0)
        numZeros = 0;
    for (int i = 0; i < numZeros; i++)
        (*samples)[i] = 0.0;
    for (int i = numZeros; i < numSamples; i++)
        (*samples)[i] = _outSamples[i - numZeros];
        
    return numSamples - numZeros;
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
OverlapAdd::processFFT(vector<complex<float> > *compBuf)
{
    for (int i = 0; i < _processors.size(); i++)
    {
        OverlapAddProcessor *processor = _processors[i];
        processor->processFFT(compBuf);
    }
}

void
OverlapAdd::processOutSamples(vector<float> *buff)
{
    for (int i = 0; i < _processors.size(); i++)
    {
        OverlapAddProcessor *processor = _processors[i];
        processor->processOutSamples(buff);
    }

    int size = _outSamples.size();
    _outSamples.resize(size + _fftSize / _overlap);
    memcpy(&_outSamples.data()[size], buff->data(), (_fftSize / _overlap) * sizeof(float));
}

void
OverlapAdd::makeWindows()
{    
    _anaWin.resize(_fftSize);
    Window::makeWindowHann(&_anaWin);

    _synthWin.resize(_fftSize);
    Window::makeWindowHann(&_synthWin);
}
