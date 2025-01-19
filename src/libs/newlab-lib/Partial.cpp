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

#include "Partial.h"

// Kalman
// "How much do we expect to our measurement vary"
//
#define PT5_KF_E_MEA 0.01 // 200.0Hz
#define PT5_KF_E_EST PT5_KF_E_MEA

// "usually a small number between 0.001 and 1"
//
// If too low: predicted values move too slowly
// If too high: predicted values go straight
//
#define PT5_KF_Q 5.0


unsigned long Partial::_currentId = 0;

Partial::Partial()
: _kf(PT5_KF_E_MEA, PT5_KF_E_EST, PT5_KF_Q)
{
    _peakIndex = 0;
    _leftIndex = 0;
    _rightIndex = 0;
    
    _freq = 0.0;
    _amp = 0.0;    
    _phase = 0.0;
    
    _state = ALIVE;
    
    _id = -1;
    _linkedId = -1;
    
    _wasAlive = false;
    _zombieAge = 0;
    
    _age = 0;
    
    _cookie = 0.0;

    _binIdxF = 0.0;
    _alpha0 = 0.0;
    _beta0 = 0.0;
}

    
Partial::Partial(const Partial &other)
: _kf(other._kf)
{
    _peakIndex = other._peakIndex;
    _leftIndex = other._leftIndex;
    _rightIndex = other._rightIndex;
    
    _freq = other._freq;
    _amp = other._amp;
    
    _phase = other._phase;
        
    _state = other._state;
        
    _id = other._id;
    _linkedId = other._linkedId;
    
    _wasAlive = other._wasAlive;
    _zombieAge = other._zombieAge;
    
    _age = other._age;
    
    _cookie = other._cookie;

    // QIFFT
    _binIdxF = other._binIdxF;
    _alpha0 = other._alpha0;
    _beta0 = other._beta0;
}

Partial::~Partial() {}

void
Partial::genNewId()
{
    _id = _currentId++;
}
    
bool
Partial::freqLess(const Partial &p1, const Partial &p2)
{
    return (p1._freq < p2._freq);
}

bool
Partial::ampLess(const Partial &p1, const Partial &p2)
{
    return (p1._amp < p2._amp);
}

bool
Partial::idLess(const Partial &p1, const Partial &p2)
{
    return (p1._id < p2._id);
}

bool
Partial::cookieLess(const Partial &p1, const Partial &p2)
{
    return (p1._cookie < p2._cookie);
}
