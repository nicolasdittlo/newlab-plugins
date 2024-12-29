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

#include "Defines.h"
#include "Utils.h"
#include "CMA2Smoother.h"

#include "TransientLib.h"

TransientLib::TransientLib()
{
    int bufferSize = 2048;
    int winSize = 5;
    
    _smoother = new CMA2Smoother(bufferSize, winSize);
}

TransientLib::~TransientLib()
{
    delete _smoother;
}

void
TransientLib::computeTransientness(const vector<float> &magns,
                                   const vector<float> &phases,
                                   const vector<float> *prevPhases,
                                   float freqAmpRatio,
                                   float smoothFactor,
                                   float sampleRate,
                                   vector<float> *transientness)
{
#define DB_THRESHOLD_TR -64.0
    
#define DB_EPS_TR 1e-15
    
#define TRANS_COEFF_GLOBAL_TR 0.5
#define TRANS_COEFF_FREQ_TR 3.0
#define TRANS_COEFF_AMP_TR 1.0
    
    transientness->resize(phases.size());
    Utils::fillZero(transientness);
    
    vector<float> &transientnessS = _tmpBuf0;
    transientnessS.resize(phases.size());
    Utils::fillZero(&transientnessS);
    
    vector<float> &transientnessP = _tmpBuf1;
    transientnessP.resize(phases.size());
    Utils::fillZero(&transientnessP);
    
    vector<int> &sampleIds = _tmpBuf2;
    Utils::FftIdsToSamplesIds(phases, &sampleIds);

    int sampleIdsSize = sampleIds.size();
    int *sampleIdsBuf = sampleIds.data();
    const float *magnsBuf = magns.data();
    
    float DB_THRESHOLD_TR_INV = 1.0/DB_THRESHOLD_TR;
    float TRANS_COEFF_FREQ_TR_GLOBAL =
        TRANS_COEFF_FREQ_TR*TRANS_COEFF_GLOBAL_TR;
    
    const float *phasesBuf = phases.data();
    
    float TRANS_COEFF_AMP_TR_GLOBAL =
        TRANS_COEFF_AMP_TR*TRANS_COEFF_GLOBAL_TR;

    float *transientnessSBuf = transientnessS.data();
    float *transientnessPBuf = transientnessP.data();
    
    for (int i = 0; i < sampleIdsSize; i++)
    {
        int sampleId = sampleIdsBuf[i];
        
        // Do as Werner Van Belle
        float magn = magnsBuf[i];
        
        // Ignore small magns
        float magnDB = Utils::ampToDB(magn,
                                      (float)DB_EPS_TR,
                                      (float)DB_THRESHOLD_TR);
        if (magnDB <= DB_THRESHOLD_TR)
            continue;
        
        float freqWeight = 0.0;
        
        // Do as Werner Van Belle
        float wf = -(magnDB - DB_THRESHOLD_TR)*DB_THRESHOLD_TR_INV;
        
        wf *= TRANS_COEFF_FREQ_TR_GLOBAL;
        
        freqWeight = wf;
        
        float ampWeight = 0.0;
        
        if ((prevPhases != NULL) && (prevPhases->size() == sampleIdsSize))
        {
            int prevPhasesSize = prevPhases->size();
            const float *prevPhasesBuf = prevPhases->data();
    
            // Use additional method: compute derivative of phase over time
            // This is a very good indicator of transientness !
            
            float phase0 = prevPhasesBuf[i];
            float phase1 = phasesBuf[i];
            
            // Ensure that phase1 is greater than phase0
            while(phase1 < phase0)
                phase1 += TWO_PI;
            
            float delta = phase1 - phase0;
            
            delta = fmod(delta, TWO_PI);
            
            if (delta > M_PI)
                //delta = 2.0*M_PI - delta;
                delta = TWO_PI - delta;
            
            //float w = delta/M_PI;
            float w = delta*M_PI_INV;
            
            w *= TRANS_COEFF_AMP_TR_GLOBAL;
            
            ampWeight = w;
        }
        
        transientnessSBuf[sampleId] += freqWeight;
        transientnessPBuf[sampleId] += ampWeight;
    }
    
    // At the end, the transientness is reversed
    // So reverse back...
    Utils::reverse(&transientnessS);
    Utils::reverse(&transientnessP);
    
    // CMA
    // Works well
    
#define NATIVE_BUFFER_SIZE_TR 2048
    float bufCoeff = ((float)phases.size())/NATIVE_BUFFER_SIZE_TR;
    
    smoothTransients(&transientnessS, smoothFactor);
    smoothTransients(&transientnessP, smoothFactor);
    
    Utils::multValue(&transientnessS, bufCoeff);
    Utils::multValue(&transientnessP, bufCoeff);
    
    int transientnessSize = transientness->size();
    float *transientnessBuf = transientness->data();
    
    transientnessSBuf = transientnessS.data();
    transientnessPBuf = transientnessP.data();
    
    for (int i = 0; i < transientnessSize; i++)
    {
        float ts = transientnessSBuf[i];
        float tp = transientnessPBuf[i];
        
        float a = tp - ts;
        if (a < 0.0)
            a = 0.0;
        
        float b = ts;
        
        // HACK
        // With that, the global volume does not increase, compared to bypass
        b *= 0.5;
                
        transientnessBuf[i] = freqAmpRatio*a + (1.0 - freqAmpRatio)*b;
    }
    
    Utils::clipMin(transientness, (float)0.0);
}

void
TransientLib::smoothTransients(vector<float> *transients,
                               float smoothFactor)
{
    // Smooth if necessary
    if (smoothFactor > 0.0)
    {
#define SMOOTH_FACTOR_TR 4.0
        
        vector<float> &smoothTransients = _tmpBuf3;
        smoothTransients.resize(transients->size());
        
        float cmaCoeff = smoothFactor*transients->size()/SMOOTH_FACTOR_TR;
        
        int cmaWindowSize = (int)cmaCoeff;
        
        _smoother->processOne(transients->data(),
                              smoothTransients.data(),
                              transients->size(),
                              cmaWindowSize);
        
        *transients = smoothTransients;
    }
    
    // After smoothing, we can have negative values
    Utils::clipMin(transients, (float)0.0);
}
