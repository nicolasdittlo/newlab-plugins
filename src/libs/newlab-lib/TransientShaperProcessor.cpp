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
#include "OverlapAdd.h"
#include "Utils.h"
#include "TransientLib.h"

#include "TransientShaperProcessor.h"

// Detection + correction
#define TRANSIENTNESS_COEFF 5.0

TransientShaperProcessor::TransientShaperProcessor(float sampleRate)
{
    _sampleRate = sampleRate;
    
    _transLib = new TransientLib();
    
    _precision = 0.0;
    _softHard = 0.0;
    _freqAmpRatio = 0.5;
}

TransientShaperProcessor::~TransientShaperProcessor()
{
    delete _transLib;
}

void
TransientShaperProcessor::reset(float sampleRate)
{
    _sampleRate = sampleRate;
}

void
TransientShaperProcessor::setPrecision(float precision)
{
    _precision = precision;
}

void
TransientShaperProcessor::setSoftHard(float softHard)
{
    _softHard = softHard;
}

void
TransientShaperProcessor::setFreqAmpRatio(float ratio)
{
    _freqAmpRatio = ratio;
}

void
TransientShaperProcessor::
processFFT(vector<complex<float> > *ioBuffer)
{
    if (fabs(_softHard) < NL_EPS)
        return;
    
    // Seems hard to take half of fft, since we work in sample space too...
    
    vector<complex<float> > &fftBuffer = _tmpBuf1;

    vector<complex<float> > bufferResized = *ioBuffer;
    bufferResized.resize(bufferResized.size() - 1);
    Utils::fillSecondFftHalf(bufferResized, &fftBuffer);
    
    vector<float> &magns = _tmpBuf2;
    vector<float> &phases = _tmpBuf3;
    Utils::complexToMagnPhase(&magns, &phases, fftBuffer);

    // Fix magnitudes ammplitudes for TransientLib
    Utils::multValue(&magns, magns.size()/4);
        
    // Compute the transientness
    vector<float> &transientness = _tmpBuf4;

    _transLib->computeTransientness(magns, phases,
                                    &_prevPhases,
                                    _freqAmpRatio,
                                    1.0 - _precision,
                                    _sampleRate,
                                    &transientness);
    
    _transientness = transientness;
    Utils::multValue(&_transientness, (float)TRANSIENTNESS_COEFF);
    
    _prevPhases = phases;
}

void
TransientShaperProcessor::processSamples(vector<float> *ioBuffer)
{
    if (fabs(_softHard) < NL_EPS)
        return;
    
    applyTransientness(ioBuffer, _transientness);
}

void
TransientShaperProcessor::
getTransientness(vector<float> *outTransientness)
{
    *outTransientness = _transientness;
}

float
TransientShaperProcessor::computeMaxTransientness()
{
#define MAX_GAIN 50.0
#define MAX_GAIN_CLIP 6.0

    // Just to be sure to not reach exactly 1.0 in the samples
#define FACTOR 0.999
    
    if (std::fabs(_softHard) < NL_EPS)
      return 1.0*FACTOR;
    
    float maxTransDB = -MAX_GAIN_CLIP/_softHard;
    
    float maxTrans = Utils::DBToAmp(maxTransDB);
    
    return maxTrans*FACTOR;
}


void
TransientShaperProcessor::applyTransientness(vector<float> *ioSamples,
                                             const vector<float> &transientness)
{
    if (transientness.size() != ioSamples->size())
        return;
    
    vector<float> &trans = _tmpBuf9;
    trans = transientness;
    
    // Avoid clipping (intelligently)
    float maxTransientness = computeMaxTransientness();
    Utils::clipMax(&trans, maxTransientness);
    
    float gainDB = MAX_GAIN*_softHard;
    
    vector<float> &gainsDB = _tmpBuf10;
    gainsDB = trans;
    Utils::multValue(&gainsDB, gainDB);
    
    vector<float> &gains = _tmpBuf11;
    gains = gainsDB;
    Utils::DBToAmp(&gains);
    
    Utils::multBuffers(ioSamples, gains);
}
