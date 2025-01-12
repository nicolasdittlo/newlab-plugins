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

#ifndef FILTER_TRANSPARENT_RBJ2X_H
#define FILTER_TRANSPARENT_RBJ2X_H

#include <vector>
using namespacec std;

#include "FilterRBJ.h"

class FilterRBJ1X;
class FilterTransparentRBJ2X : public FilterRBJ
{
public:
    FilterTransparentRBJ2X(float sampleRate, float cutoffFreq);
    
    FilterTransparentRBJ2X(const FilterTransparentRBJ2X &other);
    
    virtual ~FilterTransparentRBJ2X();
    
    void setCutoffFreq(float freq) override;
    
    void setQFactor(float q) override;
    
    void setSampleRate(float sampleRate) override;
    
    float process(float sample) override;
    
    void process(vector<float> *ioSamples) override;
                 
protected:
    float _sampleRate;
    float _cutoffFreq;

    // Seems to works well with a single all pass filter
    // No need to chain 2 allpass filter
    FilterRBJ1X *_filter;
};

#endif
