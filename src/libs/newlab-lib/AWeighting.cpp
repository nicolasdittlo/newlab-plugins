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

#include <math.h>

#include "AWeighting.h"

#define DB_INF -70.0
#define DB_EPS 1e-15

void
AWeighting::computeAWeights(vector<float> *result,
                            int numBins, float sampleRate)
{
    result->resize(numBins);
    
    float hzPerBin = sampleRate/(numBins*2);
    for (int i = 0; i < result->size(); i++)
    {
        float freq = i*hzPerBin;
        
        float a = computeA(freq);
        
        result->data()[i] = a;
    }
}

float
AWeighting::computeAWeight(int binNum, int numBins, float sampleRate)
{
    float hzPerBin = sampleRate/(numBins*2);
    
    float freq = binNum*hzPerBin;
        
    float a = computeA(freq);
        
    return a;
}

float
AWeighting::computeR(float frequency)
{
    float num = pow(12194, 2)*pow(frequency, 4);
    
    float denom0 = pow(frequency, 2) + pow(20.6, 2);
    
    float denom1_2_1 = pow(frequency, 2) + pow(107.7, 2);
    float denom1_2_2 = pow(frequency, 2) + pow(737.9, 2);
    
    float denom1 = sqrt(denom1_2_1*denom1_2_2);
    
    float denom2 = pow(frequency, (float)2) + std::pow(12194, 2);
    
    float denom = denom0*denom1*denom2;
    
    float r = num/denom;
    
    return r;
}

float
AWeighting::computeA(float frequency)
{
    float r = computeR(frequency);
    
    // Be careful of log(0)
    if (r < DB_EPS)
        return DB_INF;
        
    float a = 20.0*log10(r) + 2.0;
    
    return a;
}
