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

#include "FilterRBJ1X.h"
#include "FilterTransparentRBJ2X.h"

FilterTransparentRBJ2X::FilterTransparentRBJ2X(float sampleRate,
                                               float cutoffFreq)
{
    _sampleRate = sampleRate;
    _cutoffFreq = cutoffFreq;
    
    _filter = new FilterRBJ1X(FILTER_TYPE_ALLPASS,
                              sampleRate, cutoffFreq);
}

FilterTransparentRBJ2X::FilterTransparentRBJ2X(const FilterTransparentRBJ2X &other)
{
    _sampleRate = other._sampleRate;
    _cutoffFreq = other._cutoffFreq;
    
    _filter = new FilterRBJ1X(FILTER_TYPE_ALLPASS, _sampleRate, _cutoffFreq);
}

FilterTransparentRBJ2X::~FilterTransparentRBJ2X()
{
    delete _filter;
}

void
FilterTransparentRBJ2X::setCutoffFreq(float freq)
{
    _cutoffFreq = freq;
    
    _filter->setCutoffFreq(freq);
}

void
FilterTransparentRBJ2X::setQFactor(float q)
{
    _filter->setQFactor(q);
}

void
FilterTransparentRBJ2X::setSampleRate(float sampleRate)
{
    _sampleRate = sampleRate;
    
    _filter->setSampleRate(sampleRate);
}

float
FilterTransparentRBJ2X::process(float sample)
{
    float result = _filter->process(sample);
    
    return result;
}

void
FilterTransparentRBJ2X::process(vector<float> *ioSamples)
{    
    _filter->process(ioSamples);
}
