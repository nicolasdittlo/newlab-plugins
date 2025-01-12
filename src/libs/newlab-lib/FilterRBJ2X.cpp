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

#include "FilterRBJ2X.h"

FilterRBJ2X::FilterRBJ2X(int type,
                         float sampleRate,
                         float cutoffFreq)
{
    _type = type;
    _sampleRate = sampleRate;
    _cutoffFreq = cutoffFreq;
    
    _qFactor = 0.707;
    
    _filter = new CFxRbjFilter2X();
    
    calcFilterCoeffs();
}

FilterRBJ2X::FilterRBJ2X(const FilterRBJ2X &other)
{
    _type = other._type;
    _sampleRate = other._sampleRate;
    _cutoffFreq = other._cutoffFreq;

    _qFactor = 0.707;
    
    _filter = new CFxRbjFilter2X();
    
    calcFilterCoeffs();
}

FilterRBJ2X::~FilterRBJ2X()
{
    delete _filter;
}

void
FilterRBJ2X::setCutoffFreq(float freq)
{
    _cutoffFreq = freq;
    
    calcFilterCoeffs();
}

void
FilterRBJ2X::setSampleRate(float sampleRate)
{
    _sampleRate = sampleRate;
    
    calcFilterCoeffs();
}

void
FilterRBJ2X::setQFactor(float q)
{
    _qFactor = q;
    
    calcFilterCoeffs();
}

float
FilterRBJ2X::process(float sample)
{    
    sample = _silter->filter(sample);
        
    return sample;
}

void
FilterRBJ2X::calcFilterCoeffs()
{
    // For flat crossover
    float QFactor = _qFactor;
    
    float dbGain = 0.0;
    bool qIsBandwidth = false;
        
    _filter->calc_filter_coeffs(_type, _cutoffFreq, _sampleRate,
                                QFactor, dbGain, qIsBandwidth);
}

void
FilterRBJ2X::process(vector<float> *ioSamples)
{    
    _filter->filter(ioSamples->data(), ioSamples->size());
}
