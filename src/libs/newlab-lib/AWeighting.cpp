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

#include <BLTypes.h>

#include "AWeighting.h"

#define DB_INF -70.0
#define DB_EPS 1e-15

void
AWeighting::ComputeAWeights(WDL_TypedBuf<BL_FLOAT> *result,
                            int numBins, BL_FLOAT sampleRate)
{
    result->Resize(numBins);
    
    BL_FLOAT hzPerBin = sampleRate/(numBins*2);
    for (int i = 0; i < result->GetSize(); i++)
    {
        BL_FLOAT freq = i*hzPerBin;
        
        BL_FLOAT a = ComputeA(freq);
        
        result->Get()[i] = a;
    }
}

BL_FLOAT
AWeighting::ComputeAWeight(int binNum, int numBins, BL_FLOAT sampleRate)
{
    BL_FLOAT hzPerBin = sampleRate/(numBins*2);
    
    BL_FLOAT freq = binNum*hzPerBin;
        
    BL_FLOAT a = ComputeA(freq);
        
    return a;
}

BL_FLOAT
AWeighting::ComputeR(BL_FLOAT frequency)
{
    BL_FLOAT num = std::pow(12194, 2)*pow(frequency, 4);
    
    BL_FLOAT denom0 = std::pow(frequency, 2) + std::pow(20.6, 2);
    
    BL_FLOAT denom1_2_1 = std::pow(frequency, 2) + std::pow(107.7, 2);
    BL_FLOAT denom1_2_2 = std::pow(frequency, 2) + std::pow(737.9, 2);
    
    BL_FLOAT denom1 = std::sqrt(denom1_2_1*denom1_2_2);
    
    BL_FLOAT denom2 = std::pow(frequency, (BL_FLOAT)2) + std::pow(12194, 2);
    
    BL_FLOAT denom = denom0*denom1*denom2;
    
    BL_FLOAT r = num/denom;
    
    return r;
}

BL_FLOAT
AWeighting::ComputeA(BL_FLOAT frequency)
{
    BL_FLOAT r = ComputeR(frequency);
    
    // Be careful of log(0)
    if (r < DB_EPS)
        return DB_INF;
        
    //BL_FLOAT a = 20.0*std::log(r)/std::log(10.0) + 2.0;
    BL_FLOAT a = 20.0*std::log10(r) + 2.0;
    
    return a;
}
