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

#ifndef TRANSIENT_SHAPER_PROCESSOR_H
#define TRANSIENT_SHAPER_PROCESSOR_H

#include "OverlapAdd.h"

class TransientLib;
class TransientShaperProcessor : public OverlapAddProcessor
{
public:
    TransientShaperProcessor(float sampleRate);
    
    virtual ~TransientShaperProcessor();

    void reset(float sampleRate);
    
    void setPrecision(float precision);
    
    void setSoftHard(float softHard);
    
    void setFreqAmpRatio(float ratio);
    
    void processFFT(vector<complex<float> > *ioBuffer) override;

    void processSamples(vector<float> *ioBuffer) override;
        
    void getTransientness(vector<float> *outTransientness);
    
    void applyTransientness(vector<float> *ioSamples,
                            const vector<float> &currentTransientness);

protected:
    float computeMaxTransientness();

    float _sampleRate;
    
    float _softHard;
    float _precision;
    
    float _freqAmpRatio;
    
    vector<float> _transientness;
    
    // For computing derivative (for amp to trans)
    vector<float> _prevPhases;
        
    TransientLib *_transLib;
    
private:
    // Tmp buffers
    vector<float> _tmpBuf0;
    vector<complex<float> > _tmpBuf1;
    vector<float> _tmpBuf2;
    vector<float> _tmpBuf3;
    vector<float> _tmpBuf4;
    vector<float> _tmpBuf5;
    vector<float> _tmpBuf6;
    vector<float> _tmpBuf7;
    vector<float> _tmpBuf8;
    vector<float> _tmpBuf9;
    vector<float> _tmpBuf10;
    vector<float> _tmpBuf11;
    vector<float> _tmpBuf12;
    vector<float> _tmpBuf13;
    vector<float> _tmpBuf14;
};

#endif
