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

#ifndef CMA_SMOOTHER_H
#define CMA_SMOOTHER_H

// Central moving average smoother
class CMASmoother
{
public:
    CMASmoother(int bufferSize, int windowSize);
    
    virtual ~CMASmoother();

    void reset();

    void reset(int bufferSize, int windowSize);
    
    // Return true if nFrames has been returned
    bool process(const float *data, float *smoothedData, int nFrames);

    bool processOne(const float *data, float *smoothedData,
                    int nFrames, int windowSize);
    
protected:
    bool processInternal(const float *data, float *smoothedData, int nFrames);
    
    // Return true if something has been processed
    bool centralMovingAverage(vector<float> &inData,
                              vector<float> &outData, int windowSize);
    
    void manageConstantValues(const float *data, int nFrames);

    int _bufferSize;
    int _windowSize;
    
    bool _firstTime;
    
    float _prevVal;
    
    vector<float> _inData;
    vector<float> _outData;

private:
    // Tmp buffers
    vector<float> _tmpBuf0;
    vector<float> _tmpBuf1;
};

#endif
