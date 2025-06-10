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

#include <math.h>

#include <juce_dsp/juce_dsp.h>

#include "Window.h"
#include "Utils.h"
#include "MultiOutOverlapAdd.h"

// Must be greater or equal to _fftSize*2
#define MAX_BLOCK_SIZE 16384*16

// MultiOutOverlapAddProcessor
MultiOutOverlapAddProcessor::MultiOutOverlapAddProcessor() {}

MultiOutOverlapAddProcessor::~MultiOutOverlapAddProcessor() {}

void
MultiOutOverlapAddProcessor::processFFT(const vector<complex<float> > &inCompBuf,
                                        vector<vector<complex<float> > > *outCompBuf) {}

void
MultiOutOverlapAddProcessor::processSamples(vector<vector<float> > *buffs) {}

// MultiOutOverlapAdd
MultiOutOverlapAdd::MultiOutOverlapAdd(int fftSize, int overlap, int numOutputs, bool fft, bool ifft)
: _overlap(overlap), _fftFlag(fft), _ifftFlag(ifft)
{
    _numOutputs = numOutputs;
    
    setFftSize(fftSize);

    _outSamples.resize(_numOutputs);
}

MultiOutOverlapAdd::~MultiOutOverlapAdd() {}

void
MultiOutOverlapAdd::setFftSize(int fftSize)
{
    _fftSize = fftSize;

    _forwardFFT = std::make_unique<juce::dsp::FFT>(log2(fftSize));
    _backwardFFT = std::make_unique<juce::dsp::FFT>(log2(fftSize));

    vector<float> zeros;
    zeros.resize(MAX_BLOCK_SIZE);
    memset(zeros.data(), 0, zeros.size() * sizeof(float));

    _circSampBufsIn.setCapacity(MAX_BLOCK_SIZE);

    _circSampBufsOut.resize(_numOutputs);
    for (int i = 0; i < _numOutputs; i++)
    {
        _circSampBufsOut[i].setCapacity(MAX_BLOCK_SIZE);
        _circSampBufsOut[i].push(zeros.data(), zeros.size());
    }
    
    _tmpSampBufIn.resize(_fftSize);

    _tmpSampBufIn2.resize(_numOutputs);
    for (int i = 0; i < _numOutputs; i++)
        _tmpSampBufIn2[i].resize(_fftSize);

    _tmpSampBufOut.resize(_fftSize);
    _tmpCompBufIn.resize(_fftSize / 2 + 1);

    _tmpCompBufOut.resize(_numOutputs);
    for (int i = 0; i < _numOutputs; i++)
        _tmpCompBufOut[i].resize(_fftSize / 2 + 1);
    
    makeWindows();
}

int
MultiOutOverlapAdd::getFftSize()
{
    return _fftSize;
}

void
MultiOutOverlapAdd::setOverlap(int overlap)
{
    _overlap = overlap;

    vector<float> zeros;
    zeros.resize(MAX_BLOCK_SIZE);
    memset(zeros.data(), 0, zeros.size() * sizeof(float));

    _circSampBufsIn.setCapacity(MAX_BLOCK_SIZE);

    _circSampBufsOut.resize(_numOutputs);
    for (int i = 0; i < _numOutputs; i++)
    {
        _circSampBufsOut[i].setCapacity(MAX_BLOCK_SIZE);
        _circSampBufsOut[i].push(zeros.data(), zeros.size());
    }

    makeWindows();
}

int
MultiOutOverlapAdd::getOverlap()
{
    return _overlap;
}

void
MultiOutOverlapAdd::addProcessor(MultiOutOverlapAddProcessor *processor)
{
    _processors.push_back(processor);
}

void
MultiOutOverlapAdd::feed(const vector<float> &samples)
{
    _circSampBufsIn.push(samples.data(), samples.size());

    while (_circSampBufsIn.getSize() > _fftSize)
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

            //for (int k = 0; k < _tmpSampBufIn.size(); k++)
            //    fftInput[k] = _tmpSampBufIn[k];
            memcpy(fftInput.get(), _tmpSampBufIn.data(), _tmpSampBufIn.size()*sizeof(float));
            
            // Apply FFT
            _forwardFFT->performRealOnlyForwardTransform(fftInput.get(), true);
            
            // Store output in temporary buffer
            for (int k = 0; k < _tmpCompBufIn.size(); k++)
                _tmpCompBufIn[k] = complex(fftInput[k*2], fftInput[k*2 + 1]);
        }

        // Apply analysis coeff
        // Because fftw3 seems to scale the data when doint forward fft
        float anaCoeff = 2.0 / (_fftSize / _overlap);
        for (int k = 0; k < _tmpCompBufIn.size(); k++)
            _tmpCompBufIn[k] *= anaCoeff;
        
        // Apply callback
        _tmpCompBufOut.resize(_numOutputs);
        for (int i = 0; i < _numOutputs; i++)
            _tmpCompBufOut[i] = _tmpCompBufIn;
        
        processFFT(_tmpCompBufIn, &_tmpCompBufOut);
        
        if (_ifftFlag)
        {
            for (int i = 0; i < _numOutputs; i++)
            {
                vector<complex<float> > &tmpCompBufOutI = _tmpCompBufOut[i];
                
                // Convert to JUCE real format for inverse FFT
                juce::HeapBlock<float> ifftInput(_fftSize * 2);
                for (int k = 0; k < tmpCompBufOutI.size(); k++)
                {
                    ifftInput[k*2] = tmpCompBufOutI[k].real();
                    ifftInput[k*2 + 1] = tmpCompBufOutI[k].imag();
                }

                // Apply inverse FFT
                _backwardFFT->performRealOnlyInverseTransform(ifftInput.get());

                vector<float> &tmpSampBufIn2I = _tmpSampBufIn2[i];
                                
                // Convert back to real samples

                //for (int k = 0; k < tmpSampBufIn2I.size(); k++)
                //    tmpSampBufIn2I[k] = ifftInput[k];
                memcpy(tmpSampBufIn2I.data(), ifftInput.get(), tmpSampBufIn2I.size()*sizeof(float));
                
                // Apply resynth coeff
                //float resynthCoeff = 1.0 / _fftSize;
                //for (int k = 0; k < _tmpSampBufIn.size(); k++)
                //    _tmpSampBufIn[k] *= resynthCoeff;

                // Apply resynth coeff
                float resynthCoeff = 0.66*_fftSize / 2.0;
                for (int k = 0; k < tmpSampBufIn2I.size(); k++)
                    tmpSampBufIn2I[k] *= resynthCoeff;
            }
            
            processSamples(&_tmpSampBufIn2);

            for (int i = 0; i < _numOutputs; i++)
            {
                vector<float> &tmpSampBufIn2I = _tmpSampBufIn2[i];
                
                // Apply synthesis window
                for (int k = 0; k < tmpSampBufIn2I.size(); k++)
                    tmpSampBufIn2I[k] *= _synthWin[k];
            
                // Output
                _circSampBufsOut[i].peek(_tmpSampBufOut.data(),
                                         _synthWin.size());

                for (int k = 0; k < tmpSampBufIn2I.size(); k++)
                    tmpSampBufIn2I[k] += _tmpSampBufOut[k];

                _circSampBufsOut[i].poke(tmpSampBufIn2I.data(),
                                         _synthWin.size());
            
                _circSampBufsOut[i].pop(_fftSize / _overlap);

                _tmpSynthZeroBuf.resize(_fftSize / _overlap);
                memset(_tmpSynthZeroBuf.data(), 0, _tmpSynthZeroBuf.size() * sizeof(float));
            
                _circSampBufsOut[i].push(_tmpSynthZeroBuf.data(),
                                         _tmpSynthZeroBuf.size());
            
                int size = _outSamples[i].size();
                _outSamples[i].resize(size + _fftSize / _overlap);
                memcpy(&_outSamples[i].data()[size], tmpSampBufIn2I.data(), _fftSize / _overlap * sizeof(float));
            }
        }
    }
}

int
MultiOutOverlapAdd::getOutSamples(vector<vector<float> > *samples, int numSamples)
{
    samples->resize(_numOutputs);
    
    int numZeros = 0;
    for (int i = 0; i < _numOutputs; i++)
    {
        (*samples)[i].resize(numSamples);
        
        numZeros = numSamples - _outSamples[i].size();
        if (numZeros < 0)
            numZeros = 0;
        for (int k = 0; k < numZeros; k++)
            (*samples)[i][k] = 0.0;
        for (int k = numZeros; k < numSamples; k++)
            (*samples)[i][k] = _outSamples[i][k - numZeros];
    }
    
    return numSamples - numZeros;
}

void
MultiOutOverlapAdd::clearOutSamples()
{
    for (int i = 0; i < _numOutputs; i++)
        _outSamples[i].clear();
}

void
MultiOutOverlapAdd::flushOutSamples(int numToFlush)
{
    for (int i = 0; i < _numOutputs; i++)
    {
        if (numToFlush > _outSamples[i].size())
        {
            _outSamples[i].clear();
            continue;
        }

        _outSamples[i].erase(_outSamples[i].begin(), _outSamples[i].begin() + numToFlush);
    }
}

void
MultiOutOverlapAdd::processFFT(const vector<complex<float> > &inCompBuf,
                               vector<vector<complex<float> > > *outCompBufs)
{
    for (int i = 0; i < _processors.size(); i++)
    {
        MultiOutOverlapAddProcessor *processor = _processors[i];
        processor->processFFT(inCompBuf, outCompBufs);
    }
}

void
MultiOutOverlapAdd::processSamples(vector<vector<float> > *buffs)
{
    for (int i = 0; i < _processors.size(); i++)
    {
        MultiOutOverlapAddProcessor *processor = _processors[i];
        processor->processSamples(buffs);
    }
}

void
MultiOutOverlapAdd::makeWindows()
{    
    _anaWin.resize(_fftSize);
    Window::makeWindowHann(&_anaWin);
    
    _synthWin.resize(_fftSize);
    Window::makeWindowHann(&_synthWin);

    // Calculate combined contributions
    vector<float> combinedWindow(_fftSize, 0.0f);
    int hopSize = _fftSize / _overlap; // Hop size for overlap-add

    for (int frame = 0; frame < _overlap; ++frame)
    {
        int startIndex = frame * hopSize; // Starting index for the current frame
        for (int i = 0; i < _fftSize; ++i)
        {
            int wrappedIndex = (startIndex + i) % _fftSize; // Wrap around for circular buffer
            combinedWindow[wrappedIndex] += _synthWin[i];
        }
    }

    // Compute the normalization factor (maximum combined contribution)
    float normalizationFactor = *std::max_element(combinedWindow.begin(), combinedWindow.end());
    
    Utils::multValue(&_anaWin, 1.0 / normalizationFactor);
    Utils::multValue(&_synthWin, 1.0 / normalizationFactor);
}
