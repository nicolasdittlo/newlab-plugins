/* Copyright (C) 2025 Nicolas Dittlo <newlab.plugins@gmail.com>
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

#ifndef OVERLAP_ADD_H
#define OVERLAP_ADD_H

#include <juce_dsp/juce_dsp.h>

#include "CircularBuffer.h"

class OverlapAddProcessor
{
public:
    OverlapAddProcessor();
    virtual ~OverlapAddProcessor();
    
    virtual void processFFT(vector<complex<float> > *compBuf);
    virtual void processOutSamples(vector<float> *buf);
};

class OverlapAdd
{
public:
    OverlapAdd(int fftSize, int overlap, bool fft, bool ifft);
    virtual ~OverlapAdd();

    void setFftSize(int fftSize);
    void setOverlap(int overlap);
    
    void addProcessor(OverlapAddProcessor *processor);
    
    void feed(const vector<float> &samples);

    // Return the number of samples to flush
    int getOutSamples(vector<float> *samples, int numSamples);
    void clearOutSamples();
    void flushOutSamples(int numToFlush);
    
protected:
    virtual void processFFT(vector<complex<float> > *compBuf);
    virtual void processOutSamples(vector<float> *buff);

    void makeWindows();
        
    vector<OverlapAddProcessor *> _processors;
    
    int _fftSize;
    int _overlap;

    bool _fftFlag;
    bool _ifftFlag;
    
    CircularBuffer<float> _circSampBufsIn;
    CircularBuffer<float> _circSampBufsOut;
    
    vector<float> _tmpSampBufIn;
    vector<float> _tmpSampBufOut;
    vector<complex<float> > _tmpCompBufOut;

    vector<float> _tmpSynthZeroBuf;
    
    vector<float> _anaWin;
    vector<float> _synthWin;

    vector<float> _outSamples;

    std::unique_ptr<juce::dsp::FFT> _forwardFFT;
    std::unique_ptr<juce::dsp::FFT> _backwardFFT;
};

#endif // OVERLAP_ADD_H
