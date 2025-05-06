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

#ifndef MULTI_OUT_OVERLAP_ADD_H
#define MULTI_OUT_OVERLAP_ADD_H

#include <juce_dsp/juce_dsp.h>

#include "CircularBuffer.h"

class MultiOutOverlapAddProcessor
{
public:
    MultiOutOverlapAddProcessor();
    virtual ~MultiOutOverlapAddProcessor();
    
    virtual void processFFT(const vector<complex<float> > &inCompBuf,
                            vector<vector<complex<float> > > *outCompBufs);

    // After ifft
    virtual void processSamples(vector<vector<float> > *bufs);
};

class MultiOutOverlapAdd
{
public:
    MultiOutOverlapAdd(int fftSize, int overlap, int numOutputs, bool fft, bool ifft);
    virtual ~MultiOutOverlapAdd();

    void setFftSize(int fftSize);
    void setOverlap(int overlap);
    
    void addProcessor(MultiOutOverlapAddProcessor *processor);
    
    void feed(const vector<float> &samples);

    // Return the number of samples to flush
    int getOutSamples(vector<vector<float> > *samples, int numSamples);
    void clearOutSamples();
    void flushOutSamples(int numToFlush);
    
protected:
    void processFFT(const vector<complex<float> > &inCompBuf,
                    vector<vector<complex<float> > > *outCompBufs);
    void processSamples(vector<vector<float> > *bufs);
    
    void makeWindows();
        
    vector<MultiOutOverlapAddProcessor *> _processors;
    
    int _fftSize;
    int _overlap;

    int _numOutputs;
    
    bool _fftFlag;
    bool _ifftFlag;
    
    CircularBuffer<float> _circSampBufsIn;
    vector<CircularBuffer<float> > _circSampBufsOut;
    
    vector<float> _tmpSampBufIn;
    vector<vector<float> > _tmpSampBufIn2;
    vector<float> _tmpSampBufOut;
    vector<vector<complex<float> > > _tmpCompBufOut;

    vector<float> _tmpSynthZeroBuf;
    
    vector<float> _anaWin;
    vector<float> _synthWin;

    vector<vector<float> > _outSamples;

    std::unique_ptr<juce::dsp::FFT> _forwardFFT;
    std::unique_ptr<juce::dsp::FFT> _backwardFFT;
};

#endif // MULTI_OUT_OVERLAP_ADD_H
