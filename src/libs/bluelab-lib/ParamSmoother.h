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

#ifndef PARAM_SMOOTHER_H
#define PARAM_SMOOTHER_H

#include <cmath>

#include "Defines.h"

// 140ms => coeff 0.999 at 44100Hz
#define DEFAULT_SMOOTHING_TIME_MS 140.0

// See: http://www.musicdsp.org/en/latest/Filters/257-1-pole-lpf-for-smooth-parameter-changes.html
//
// Advanced version of simple smooth usually used by me
// - the result won't vary if we change the sample rate
// - we specify the smooth with smooth time (ms)
//
class ParamSmoother
{
public:
    ParamSmoother(float sampleRate,
                  float value,
                  float smoothingTimeMs = DEFAULT_SMOOTHING_TIME_MS)
    {
        _smoothingTimeMs = smoothingTimeMs;
        _sampleRate = sampleRate;

        _z = value;
        _targetValue = value;
        
        reset(sampleRate);
    }
    
    virtual ~ParamSmoother() {}
    
    inline void reset(float sampleRate, float smoothingTimeMs = -1.0)
    {
        _sampleRate = sampleRate;

        if (smoothingTimeMs >= 0.0)
            _smoothingTimeMs = smoothingTimeMs;
        
        _a = std::exp(-(float)2.0*M_PI/
                      (_smoothingTimeMs * (float)0.001 * sampleRate));
        _b = 1.0 - _a;

        _z = _targetValue;
    }

    // Set smooth time, but without resetting
    inline void setSmoothTimeMs(float smoothingTimeMs)
    {
        _smoothingTimeMs = smoothingTimeMs;
        
        _a = std::exp(-(float)2.0*M_PI/
                      (_smoothingTimeMs * (float)0.001 * _sampleRate));
        _b = 1.0 - _a;
    }
    
    inline void setTargetValue(float val)
    {
        _targetValue = val;
    }

    inline void resetToTargetValue(float val)
    {
        _targetValue = val;
        _z = _targetValue;
    }

    inline float process()
    {
        _z = (_targetValue * _b) + (_z * _a);
        
        return _z;
    }

    // Get current value without processing
    inline float pickCurrentValue()
    {
        return _z;
    }
    
    inline bool isStable()
    {
        return (std::fabs(_z - _targetValue) < 1e-10);
    }

    // Convert ms delay to smooth factor
    static inline float
    computeSmoothFactor(float smoothingTimeMs, float sampleRate)
    {
        float factor = std::exp(-(float)2.0*M_PI/
                                   (smoothingTimeMs * (float)0.001 * sampleRate));
        return factor;
    }
    
protected:
    float _smoothingTimeMs;
    float _sampleRate;
    
    float _a;
    float _b;
    float _z;

    float _targetValue;
};

#endif
