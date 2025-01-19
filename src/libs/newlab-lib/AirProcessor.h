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

#ifndef AIR_PROCESSOR_H
#define AIR_PROCESSOR_H

#include "OverlapAdd.h"

class WienerSoftMasking;
class PartialTracker;
class AirProcessor : public OverlapAddProcessor
{
public:
    AirProcessor(int bufferSize, int overlap, float sampleRate);
    
    virtual ~AirProcessor();

    void reset();
    
    void reset(int bufferSize, int overlap, float sampleRate);
    
    void processFFT(vector<complex<float> > *ioBuffer) override;

    void setThreshold(float threshold);

    void setMix(float mix);

    void setUseSoftMasks(bool flag);

    void setEnableSum(bool flag);
    
    int getLatency();

    void getNoiseBuffer(vector<float> *ioBuffer);
    
    void getHarmoBuffer(vector<float> *ioBuffer);

    void getSumBuffer(vector<float> *ioBuffer);
        
protected:
    void detectPartials(const vector<float> &magns,
                        const vector<float> &phases);

    // Compute mask for s0, from s0 and s1
    void computeMask(const vector<float> &s0Buf,
                     const vector<float> &s1Buf,
                     vector<float> *s0Mask);

    int _bufferSize;
    int _overlap;
    float _sampleRate;
    
    PartialTracker *_partialTracker;

    float _mix;
    
    bool _useSoftMasks;
    WienerSoftMasking *_softMasking;

    vector<float> _noiseBuffer;
    vector<float> _harmoBuffer;
    vector<float> _sumBuffer;

    bool _enableComputeSum;
    
private:
    // Tmp buffers
    vector<complex<float> > _tmpBuf0;
    vector<float> _tmpBuf1;
    vector<float> _tmpBuf2;
    vector<float> _tmpBuf4;
    vector<float> _tmpBuf5;
    vector<float> _tmpBuf7;
    vector<float> _tmpBuf8;
    vector<float> _tmpBuf9;
    vector<complex<float> > _tmpBuf10[2];
    vector<complex<float> > _tmpBuf11;
    vector<complex<float> > _tmpBuf12;
    vector<complex<float> > _tmpBuf13;
    vector<complex<float> > _tmpBuf14;
    vector<complex<float> > _tmpBuf15;
    vector<float> _tmpBuf17;
    vector<complex<float> > _tmpBuf18;
    vector<complex<float> > _tmpBuf19;
    vector<complex<float> > _tmpBuf20;
    vector<complex<float> > _tmpBuf21;
    vector<float> _tmpBuf22;
};

#endif
