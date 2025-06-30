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

#ifndef FILTER_RBJ_2X_H
#define FILTER_RBJ_2X_H

#include "CFxRbjFilter.h"

#include "FilterRBJ.h"

// Chains the coefficients automatically
class FilterRBJ2X : public FilterRBJ
{
public:
    FilterRBJ2X(int type, float sampleRate, float cutoffFreq);
    
    FilterRBJ2X(const FilterRBJ2X &other);
    
    virtual ~FilterRBJ2X();
    
    void setCutoffFreq(float freq) override;
    
    void setSampleRate(float sampleRate) override;
    
    void setQFactor(float q) override;
    
    float process(float sample) override;
    
    void process(vector<float> *ioSamples) override;
                 
protected:
    void calcFilterCoeffs();
    
    int _type;
    float _sampleRate;
    float _cutoffFreq;
    float _qFactor;
    
    CFxRbjFilter2X *_filter;
};

#endif
