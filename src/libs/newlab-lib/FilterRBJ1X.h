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

#ifndef FILTER_RBJ1X_H
#define FILTER_RBJ1X_H

#include <vector>
using namespace std;

#include "CFxRbjFilter.h"
#include "FilterRBJ.h"

class FilterRBJ1X : public FilterRBJ
{
public:
    FilterRBJ1X(int type, float sampleRate, float cutoffFreq);
    
    FilterRBJ1X(const FilterRBJ1X &other);
    
    virtual ~FilterRBJ1X();
    
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
    
    CFxRbjFilter *_filter;
};

#endif
