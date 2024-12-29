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

#include <algorithm>
using namespace std;

#include "Utils.h"
#include "Defines.h"

#include "MelScale.h"

// See: http://practicalcryptography.com/miscellaneous/machine-learning/guide-mel-frequency-cepstral-coefficients-mfccs/

MelScale::MelScale() {}

MelScale::~MelScale() {}

float
MelScale::hzToMel(float freq)
{
    float mel = 2595.0*log10((float)(1.0 + freq/700.0));
    
    return mel;
}

float
MelScale::melToHz(float mel)
{
    float hz = 700.0*(pow((float)10.0, (float)(mel/2595.0)) - 1.0);
    
    return hz;
}

void
MelScale::hzToMel(vector<float> *resultMagns,
                  const vector<float> &magns,
                  float sampleRate)
{
    // For dB
    resultMagns->resize(magns.size());
    Utils::fillZero(resultMagns);
    
    float maxFreq = sampleRate*0.5;
    if (maxFreq < NL_EPS)
        return;
    
    float maxMel = hzToMel(maxFreq);
    
    float melCoeff = maxMel/resultMagns->size();
    float idCoeff = (1.0/maxFreq)*resultMagns->size();
    
    int resultMagnsSize = resultMagns->size();
    float *resultMagnsData = resultMagns->data();
    
    int magnsSize = magns.size();
    const float *magnsData = magns.data();
    for (int i = 0; i < resultMagnsSize; i++)
    {
        float mel = i*melCoeff;
        float freq = melToHz(mel);
        
        float id0 = freq*idCoeff;
        
        int id0i = (int)id0;
        
        float t = id0 - id0i;
        
        if (id0i >= magnsSize)
            continue;
        
        // NOTE: this optim doesn't compute exactly the same thing than the original version
        int id1 = id0i + 1;
        if (id1 >= magnsSize)
            continue;
        
        float magn0 = magnsData[id0i];
        float magn1 = magnsData[id1];
        
        float magn = (1.0 - t)*magn0 + t*magn1;
        
        resultMagnsData[i] = magn;
    }
}

void
MelScale::melToHz(vector<float> *resultMagns,
                  const vector<float> &magns,
                  float sampleRate)
{
    resultMagns->resize(magns.size());
    Utils::fillZero(resultMagns);
    
    float hzPerBin = sampleRate*0.5/magns.size();
    
    float maxFreq = sampleRate*0.5;
    float maxMel = hzToMel(maxFreq);
    
    int resultMagnsSize = resultMagns->size();
    float *resultMagnsData = resultMagns->data();
    int magnsSize = magns.size();
    const float *magnsData = magns.data();
    
    for (int i = 0; i < resultMagnsSize; i++)
    {
        float freq = hzPerBin*i;
        float mel = hzToMel(freq);
        
        float id0 = (mel/maxMel) * resultMagnsSize;
        
        if ((int)id0 >= magnsSize)
            continue;
        
        float t = id0 - (int)(id0);
        
        int id1 = id0 + 1;
        if (id1 >= magnsSize)
            continue;
        
        float magn0 = magnsData[(int)id0];
        float magn1 = magnsData[id1];
        
        float magn = (1.0 - t)*magn0 + t*magn1;
        
        resultMagnsData[i] = magn;
    }
}
