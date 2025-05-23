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

#ifndef CMA2_SMOOTHER_H
#define CMA2_SMOOTHER_H

#include <vector>
using namespace std;

#include "CMASmoother.h"

// Double central moving average smoother
class CMA2Smoother
{
public:
    CMA2Smoother(int bufferSize, int windowSize);
    
    virtual ~CMA2Smoother();
    
    // Return true if nFrames has been returned
    bool process(const float *data, float *smoothedData, int nFrames);

    bool processOne(const float *data,
                    float *smoothedData,
                    int nFrames, int windowSize);
    
    bool processOne(const vector<float> &inData,
                    vector<float> *outSmoothedData,
                    int windowSize);
    
    void reset();
    
protected:
    CMASmoother _smoother0;
    CMASmoother _smoother1;

    // For processOne()
    CMASmoother _smootherP1;

private:
    // Tmp buffers
    vector<float> _tmpBuf0;
};

#endif
