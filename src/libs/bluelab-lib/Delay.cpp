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

#include "Utils.h"

#include "Delay.h"

Delay::Delay(float delay)
{
    _readPos = 0;
    
    int delayI = ceil(delay);
    _delayLine.resize(delayI);
    Utils::fillZero(&_delayLine);
   
    _writePos = _delayLine.size() - 1;
    if (_writePos < 0)
        _writePos = 0;

    setDelay(delay);
}

Delay::Delay(const Delay &other)
{
    float delay = other._delay;
    
    _readPos = 0;
    
    int delayI = ceil(delay);
    _delayLine.resize(delayI);
    Utils::fillZero(&_delayLine);
    
    _writePos = _delayLine.size() - 1;
    if (_writePos < 0)
        _writePos = 0;
    
    setDelay(delay);
}

Delay::~Delay() {}

void
Delay::reset()
{
    Utils::fillZero(&_delayLine);
    
    _readPos = 0;
    _writePos = _delayLine.size() - 1;
    if (_writePos < 0)
        _writePos = 0;
}

void
Delay::setDelay(float delay)
{
    _delay = delay;
 
    int delayI = ceil(delay);
    
    if (delayI == 0)
        delayI = 1;
    
    float lastValue = 0.0;
    if (_delayLine.size() > 0)
    {
        // Must take the value of _writePos - 1
        // this is the last written value
        int writeIndex = _writePos - 1;
        if (writeIndex < 0)
            writeIndex += _delayLine.size();
        writeIndex = writeIndex % _delayLine.size();
        
        lastValue = _delayLine.data()[writeIndex];
    }
    
    int delayLinePrevSize = _delayLine.size();
    if (delayI > delayLinePrevSize)
    {
        // Insert values
        int numToInsert = delayI - delayLinePrevSize;
        Utils::insertValues(&_delayLine, _writePos,
                            numToInsert, lastValue);
        
        // Adjust read and write positions
        if (_readPos > _writePos)
            _readPos = (_readPos + numToInsert) % _delayLine.size();
        
        _writePos = (_writePos + numToInsert) % _delayLine.size();
    }
    
    if (delayI < delayLinePrevSize)
    {
        // Remove values
        int numToRemove = delayLinePrevSize - delayI;
        Utils::removeValuesCyclic(&_delayLine, _writePos, numToRemove);
        
        // Adjust the read an write positions
        if (_readPos > _writePos)
        {
            _readPos -= numToRemove;
            if (_readPos < 0)
                _readPos += _delayLine.size();
            _readPos = _readPos % _delayLine.size();
        }
        
        _writePos -= numToRemove;
        if (_writePos < 0)
            _writePos += _delayLine.size();
        _writePos = _writePos % _delayLine.size();
    }
}

float
Delay::processSample(float sample)
{
    int delaySize = _delayLine.size();
    if (delaySize == 0)
        return sample;
    
    float *delayBuf = _delayLine.data();
    
    // Read
    if (delaySize == 1)
    {
        delayBuf[delaySize - 1] = sample;
        
        return sample;
    }
    
    // Read by interpolating, to manage non integer delays
    float result0 = delayBuf[_readPos];
    
    _readPos++;
    if (_readPos >= delaySize)
        _readPos -= delaySize;
    
    float result1 = delayBuf[_readPos];
    
    float t = _delay - (int)_delay;
    
    float result = (1.0 - t)*result0 + t*result1;
    
    // Write
    delayBuf[_writePos] = sample;
    
    _writePos++;
    if (_writePos >= delaySize)
        _writePos -= delaySize;
    
    return result;
}

void
Delay::processSamples(vector<float> *samples)
{
    for (int i = 0; i < samples->size(); i++)
    {
        float samp0 = samples->data()[i];
        float samp1 = processSample(samp0);
        samples->data()[i] = samp1;
    }
}
