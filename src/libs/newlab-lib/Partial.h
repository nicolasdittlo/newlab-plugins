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

#ifndef PARTIAL_H
#define PARTIAL_H

#include <vector>
using namespace std;

#include "KalmanFilter.h"

class Partial
{
 public:
    enum State
    {
        ALIVE = 0,
        ZOMBIE,
        DEAD
    };
        
    Partial();
        
    Partial(const Partial &other);
        
    virtual ~Partial();
        
    void genNewId();
        
    static bool freqLess(const Partial &p1, const Partial &p2);
        
    static bool ampLess(const Partial &p1, const Partial &p2);
        
    static bool idLess(const Partial &p1, const Partial &p2);
        
    static bool cookieLess(const Partial &p1, const Partial &p2);

 public:
    int _peakIndex;
    int _leftIndex;
    int _rightIndex;
        
    // When detecting and filtering, _freq and _amp are "scaled and normalized"
    // After processing, we can compute the real frequencies in Hz and amp in dB.
    float _freq;
    union{
        // Inside PartialTracker
        float _amp;
            
        // After, outside PartialTracker, if external classes need amp in dB
        // Need to call denormPartials() then partialsAmpToAmpDB()
        float _ampDB;
    };
    float _phase;

    // Partial id
    long _id;
    
    // Index of the linked partial in the current array (for associating)
    long _linkedId;
    
    enum State _state;
        
    bool _wasAlive;
    long _zombieAge;
        
    long _age;
        
    // All-purpose field
    float _cookie;

    KalmanFilter _kf;

    // QIFFT
    float _binIdxF;
    float _alpha0;
    float _beta0;
    
 protected:
    static unsigned long _currentId;
};

#endif
