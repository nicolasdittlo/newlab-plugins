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

#include <string.h>

#include "Utils.h"
#include "CMASmoother.h"

CMASmoother::CMASmoother(int bufferSize, int windowSize)
{
    _bufferSize = bufferSize;
    _windowSize = windowSize;
    _firstTime = true;
    _prevVal = 0.0;
}

CMASmoother::~CMASmoother() {}

void
CMASmoother::reset()
{
    _inData.resize(0);
    _outData.resize(0);
    
    _firstTime = true;
    
    _prevVal = 0.0;
}

void
CMASmoother::reset(int bufferSize, int windowSize)
{
    _bufferSize = bufferSize;
    _windowSize = windowSize;
    
    _inData.resize(0);
    _outData.resize(0);
    
    _firstTime = true;
    
    _prevVal = 0.0;
}

bool
CMASmoother::process(const float *data, float *smoothedData, int nFrames)
{
    if (_firstTime)
        // Process one time with zeros (for continuity after).
        // Consider we have some silence before, then process it for dummy
        // Then at the next buffer, we will have the correct state, with the correct mPrevVal etc.
    {
        _firstTime = false;
        
        vector<float> zeros;
        zeros.resize(nFrames);
        for (int i = 0; i < nFrames; i++)
            zeros.data()[i] = 0.0;
        
        vector<float> smoothed;
        smoothed.resize(nFrames);
        
        processInternal(zeros.data(), smoothed.data(), nFrames);
    }
    
    // Normal processing
    bool processed = processInternal(data, smoothedData, nFrames);
    
    return processed;
}

bool
CMASmoother::processInternal(const float *data, float *smoothedData, int nFrames)
{
    manageConstantValues(data, nFrames);

    Utils::append(&_inData, data, nFrames);
    
    vector<float> outData;
    bool processed = centralMovingAverage(_inData, outData, _windowSize);
    if (processed)
    {
        // Add out data
        Utils::append(&_outData, outData.data(), outData.size()); 
        
        int outDataSize = outData.size();
        int inDataSize = _inData.size();
        
        // Consume in data
        vector<float> newInData;
        Utils::append(&newInData, &_inData.data()[outDataSize], inDataSize - outDataSize);
        _inData = newInData;
    
        if (outDataSize >= nFrames)
        {
            // Return out data
            memcpy(smoothedData, _outData.data(), nFrames*sizeof(float));
        
            // Consume out data
            vector<float> newOutData;
            Utils::append(&newOutData, &_outData.data()[nFrames], outDataSize - nFrames);
            _outData = newOutData;
        
            return true;
        }
    }
    
    return false;
}

bool
CMASmoother::processOne(const float *data, float *smoothedData,
                        int nFrames, int windowSize)
{
    if (nFrames < windowSize)
        return false;
    
    vector<float> &inData = _tmpBuf0;
    
    // First, fill with zeros
    int inDataFullSize = 3*windowSize + nFrames;
    
    if (inData.size() != inDataFullSize)
        inData.resize(inDataFullSize);
    
    Utils::fillZero(&inData, windowSize);
    
    // Copy mirrored data at the beginning and at the end to avoid zero
    // values at the bnoudaries, after smoothing
    
    // Copy mirrored data at the beginning
    int prevSize0 = windowSize;
    float *inDataBuf = inData.data();
    for (int i = 0; i < windowSize; i++)
    {
        float val = data[windowSize - i];
        
        inDataBuf[prevSize0 + i] = val;
    }
    
    int prevSize1 = 2*windowSize;
    
    Utils::copyBuf(&inData.data()[prevSize1], data, nFrames);
    
    int prevSize2 = 2*windowSize + nFrames;
    
    inDataBuf = inData.data();
    for (int i = 0; i < windowSize; i++)
    {
        float val = data[nFrames - i - 1];
        
        inDataBuf[prevSize2 + i] = val;
    }
    
    vector<float> &outData = _tmpBuf1;
    int outDataSize = inDataFullSize - 2*(windowSize/2); // FIX for odd window size
    if (outData.size() != outDataSize)
        outData.resize(outDataSize);
    
    int halfWindowSize = windowSize/2;
    float windowSizeInv = 1.0/windowSize;
    
    int outStep = 0;
    
    // Centered moving average
    float prevVal = 0.0;
    float *outDataBuf = outData.data();
    for (int i = halfWindowSize; i < inData.size() - halfWindowSize; i++)
    {
        float xn = prevVal + (inDataBuf[i + halfWindowSize] -
                              inDataBuf[i - halfWindowSize])*windowSizeInv;
        
        outDataBuf[outStep++] = xn;
        
        prevVal = xn;
    }
    
    if (outData.size() < nFrames)
        return false;
    
    // Get data from index "windowSize", this is where it begins to be valid
    // Because we add extra data at the beginning
    memcpy(smoothedData,
           &outData.data()[windowSize + halfWindowSize],
           nFrames*sizeof(float));
    
    return true;
}

bool
CMASmoother::centralMovingAverage(vector<float> &inData, vector<float> &outData, int windowSize)
{
    if (inData.size() < windowSize)
        return false;
    
    // Centered moving average
    for (int i = windowSize/2; i < inData.size() - windowSize/2; i++)
    {
        float xn = _prevVal + (inData.data()[i + windowSize/2] - inData.data()[i - windowSize/2])/windowSize;

        outData.push_back(xn);
        
        _prevVal = xn;
    }
    
    return true;
}

void
CMASmoother::manageConstantValues(const float *data, int nFrames)
{
    if (nFrames == 0)
        return;
    
    float firstValue = data[0];
    
    bool constantValue = true;
    for (int i = 1; (i < nFrames) && constantValue; i++)
        constantValue = (data[i] == firstValue);
    
    if (constantValue)
        _prevVal = (_prevVal + firstValue)/2.0;
}
