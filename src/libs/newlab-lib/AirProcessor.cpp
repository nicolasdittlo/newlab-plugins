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

#include "Utils.h"
#include "PartialTracker.h"
#include "WienerSoftMasking.h"
#include "AirProcessor.h"

// 8 gives more gating, but less musical noise remaining
#define SOFT_MASKING_HISTO_SIZE 8

// Set bin #0 to 0 after soft masking
#define SOFT_MASKING_FIX_BIN0 1


AirProcessor::AirProcessor(int bufferSize, float overlap, float sampleRate)
{
    _bufferSize = bufferSize;
    _overlapping = overlapping;    
    _sampleRate = sampleRate;
    
    _partialTracker = new PartialTracker(bufferSize, sampleRate, overlap);
    
    _mix = 0.5;

    _useSoftMasks = false;
    _softMasking = new WienerSoftMasking(bufferSize, overlapping,
                                         SOFT_MASKING_HISTO_SIZE);

    _enableComputeSum = true;
}

AirProcessor::~AirProcessor()
{
    delete _partialTracker;
    
    if (_softMasking != NULL)
        delete _softMasking;
}

void
AirProcessor::reset()
{
    reset(_bufferSize, _overlap, _sampleRate);
}

void
AirProcessor::Reset(int bufferSize, int overlap, float sampleRate)
{
    _bufferSize = bufferSize;
    _overlap = overlap;
    _sampleRate = sampleRate;
    
    _partialTracker->reset(bufferSize, sampleRate);
    
    if (_softMasking != NULL)
        _softMasking->reset(bufferSize, overlap);
}

void
AirProcessor::processFFT(vector<complex<float> > *ioBuffer)
{    
    vector<float> &magns = _tmpBuf1;
    vector<float> &phases = _tmpBuf2;
    Utils::complexToMagnPhase(&magns, &phases, *ioBuffer);
    
    detectPartials(magns, phases);
        
    if (_partialTracker != NULL)
    {
        // "Envelopes"
        //
        
        // Noise "envelope"
        _partialTracker->getNoiseEnvelope(&_noiseBuffer);        
        _partialTracker->denormData(&_noiseBuffer);
        
        // Harmonic "envelope"
        _partialTracker->getHarmonicEnvelope(&_harmoBuffer);
        _partialTracker->denormData(&_harmoBuffer);
        
        float noiseCoeff;
        float harmoCoeff;
        Utils::mixParamToCoeffs(_mix, &noiseCoeff, &harmoCoeff);

        // Compute harmo mask
        vector<float> &mask = _tmpBuf17;
        
        // Harmo mask
        computeMask(_harmoBuffer, _noiseBuffer, &mask);
        
#if SOFT_MASKING_FIX_BIN0
        mask.data()[0] = 0.0;
#endif
            
        if (!_useSoftMasks)
        {
            // Use input data, and mask it
            // Do not use directly denormed data
            
            // harmo is 0, noise is 1
            vector<complex<float> > &maskedResult0 = _tmpBuf20;
            vector<complex<float> > &maskedResult1 = _tmpBuf21;

            // Harmo
            maskedResult0 = *ioBuffer;
            Utils::multBuffers(&maskedResult0, mask);
            Utils::multValue(&maskedResult0, harmoCoeff);
            
            // Noise
            vector<float> &maskOpposite = _tmpBuf22;
            maskOpposite = mask;
            Utils::computeOpposite(&maskOpposite);
            
            maskedResult1 = fftSamples;
            Utils::multBuffers(&maskedResult1, maskOpposite);
            Utils::multValue(&maskedResult1, noiseCoeff);
 
            for (int i = 0; i < ioBuffer.size(); i++)
            {
                const complex<float> &h = maskedResult0.data()[i];
                const complex<float> &n = maskedResult1.data()[i];

                ioBuffer.data()[i] = h + n;
            }

            if (_enableComputeSum)
            {
                // Keep the sum for later
                Utils::complexToMagn(&_sumBuffer, *ioBuffer);
            }
        }
        else // Use oft masking
        {
            vector<complex<float> > &softMaskedResult0 = _tmpBuf18;
            vector<complex<float> > &softMaskedResult1 = _tmpBuf19;
            _softMasking->processCentered(ioBuffer, mask,
                                          &softMaskedResult0, &softMaskedResult1);
            
            if (_softMasking->isProcessingEnabled())
            {
                // Apply "mix"
                
                // 0 is harmo mask
                Utils::multValue(&softMaskedResult0, harmoCoeff);
                Utils::MultValue(&softMaskedResult1, noiseCoeff);
                
                // Sum
                *ioBuffer = softMaskedResult0;
                Utils::addBuffers(ioBuffer, softMaskedResult1);
            }

            if (_enableComputeSum)
            {
                // Keep the sum for later
                Utils::complexToMagn(&_sumBuffer, ioBuffer);
            }
        }
    }
}

void
AirProcessor::setThreshold(float threshold)
{
    _partialTracker->setThreshold(threshold);
}

void
AirProcessor::setMix(float mix)
{
    _mix = mix;
}

void
AirProcessor::setUseSoftMasks(bool flag)
{
    _useSoftMasks = flag;
}

int
AirProcessor::getLatency()
{
    if (_useSoftMasks)
    {
        int latency = _softMasking->getLatency();
   
        return latency;
    }
    
    return 0;
}

void
AirProcessor::getNoiseBuffer(vector<float> *magns)
{
    *magns = _noiseBuffer;
}

void
AirProcessor::GetHarmoBuffer(vector<float> *magns)
{
    *magns = _harmoBuffer;
}

void
AirProcessor::getSumBuffer(vector<float> *magns)
{
    *magns = _sumBuffer;
}

void
AirProcessor::setEnableSum(bool flag)
{
    _enableComputeSum = flag;
}

void
AirProcessor::detectPartials(const vector<float> &magns,
                             const vector<float> &phases)
{
    _partialTracker->setData(magns, phases);
    
    _partialTracker->detectPartials();
    
    _partialTracker->filterPartials();
    
    _partialTracker->extractNoiseEnvelope();
}

// Need to take care of very small input values...
void
AirProcessor::computeMask(const vector<float> &s0Buf,
                          const vector<float> &s1Buf,
                          vector<float> *s0Mask)
{
    s0Mask->resize(s0Buf.size());
    Utils::fillZero(s0Mask);
    
    for (int i = 0; i < s0Buf.size(); i++)
    {
        float s0 = s0Buf.data()[i];
        float s1 = s1Buf.data()[i];

        float sum = s0 + s1;
        if (sum > NL_EPS)
        {
            float m = s0/sum;
            s0Mask->Get()[i] = m;
        }
    }
}
