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

#ifndef STN_PROCESSOR_STEP1_H
#define STN_PROCESSOR_STEP1_H

#include <vector>
using namespace std;

#include "bl_queue.h"
#include "MultiOutOverlapAdd.h"

class STNAlgo;
class STNProcessorStep1 : public MultiOutOverlapAddProcessor
{
public:
    STNProcessorStep1(int bufferSize, int overlap, float sampleRate);
    
    virtual ~STNProcessorStep1();

    void reset();
    
    void reset(int bufferSize, int overlap, float sampleRate);
    
    void processFFT(const vector<complex<float> > &inBuffer,
                    vector<vector<complex<float> > > *outBuffers) override;
    
    int getLatency();

    void getNoiseBuffer(vector<float> *buf);
        
protected:
    int _bufferSize;
    int _overlap;
    float _sampleRate;

    bl_queue<vector<complex<float> > > _X;
    bl_queue<vector<float> > _XMagn;

    vector<float> _noiseBuffer;

    STNAlgo *_stnAlgo;
};

#endif
