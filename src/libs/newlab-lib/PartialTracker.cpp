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

#include "Defines.h"
#include "Window.h"
#include "AWeighting.h"
#include "Utils.h"
#include "PartialTracker.h"

#define MIN_AMP_DB -120.0
#define MIN_NORM_AMP 1e-15

#define PARTIALS_HISTORY_SIZE 2

// Detect partials

#define DETECT_PARTIALS_START_INDEX 2
#define DISCARD_FLAT_PARTIAL_COEFF 25000.0
#define GLUE_BARBS_AMP_RATIO    10.0

// Filter
#define MAX_ZOMBIE_AGE 2

// Seems better with 200Hz (tested on "oohoo")
#define DELTA_FREQ_ASSOC 0.01 // For normalized freqs. Around 100Hz

// Kalman
// "How much do we expect to our measurement vary"
#define PT5_KF_E_MEA 0.01 // 200.0Hz
#define PT5_KF_E_EST PT5_KF_E_MEA

// "usually a small number between 0.001 and 1"
#define PT5_KF_Q 5.0

// Expe
// 1 gives good results for "Ooohoo" (method "Min")
// 2 gives good results for "Ti Tsu Koi" (method "Min")
#define NUM_ITER_EXTRACT_NOISE 4

// Musical denoise
#define HISTORY_SIZE_MUS_NOISE 4

// We use phases interpolation
//
// See: https://www.dsprelated.com/freebooks/sasp/Spectral_Modeling_Synthesis.html
//
// and: https://www.dsprelated.com/freebooks/sasp/PARSHL_Program.html#app:parshlapp
//
// and: https://ccrma.stanford.edu/~jos/parshl/
//
// Get the precision when interpolating peak magns, but also for phases

unsigned long PartialTracker::Partial::_currentId = 0;


PartialTracker::Partial::Partial()
: _kf(PT5_KF_E_MEA, PT5_KF_E_EST, PT5_KF_Q)
{
    _peakIndex = 0;
    _leftIndex = 0;
    _rightIndex = 0;
    
    _freq = 0.0;
    _amp = 0.0;
    
    _phase = 0.0;
    
    _state = ALIVE;
    
    _id = -1;
    
    _wasAlive = false;
    _zombieAge = 0;
    
    _age = 0;
    
    _cookie = 0.0;
    
    // Kalman
    _predictedFreq = 0.0;
}

    
PartialTracker::Partial::Partial(const Partial &other)
: _kf(other._kf)
{
    _peakIndex = other._peakIndex;
    _leftIndex = other._leftIndex;
    _rightIndex = other._rightIndex;
    
    _freq = other._freq;
    _amp = other._amp;
    
    _phase = other._phase;
        
    _state = other._state;;
        
    _id = other._id;
    
    _wasAlive = other._wasAlive;
    _zombieAge = other._zombieAge;
    
    _age = other._age;
    
    _cookie = other._cookie;
    
    // Kalman
    _predictedFreq = other._predictedFreq;
}

PartialTracker::Partial::~Partial() {}

void
PartialTracker::Partial::genNewId()
{
    _id = _currentId++;
}
    
bool
PartialTracker::Partial::freqLess(const Partial &p1, const Partial &p2)
{
    return (p1._freq < p2._freq);
}

bool
PartialTracker::Partial::ampLess(const Partial &p1, const Partial &p2)
{
    return (p1._amp < p2._amp);
}

bool
PartialTracker::Partial::idLess(const Partial &p1, const Partial &p2)
{
    return (p1._id < p2._id);
}

bool
PartialTracker::Partial::cookieLess(const Partial &p1, const Partial &p2)
{
    return (p1._cookie < p2._cookie);
}

PartialTracker::PartialTracker(int bufferSize, float sampleRate)
{
    _bufferSize = bufferSize;
    _sampleRate = sampleRate;
    
    _threshold = -60.0;
    
    _maxDetectFreq = -1.0;
    
    // Scale
    _scale = new Scale();
    
    _xScale = Scale::MEL_FILTER;
    _yScale = Scale::DB;
    
    _xScaleInv = Scale::MEL_FILTER_INV;
    _yScaleInv = Scale::DB_INV;
    
    _timeSmoothCoeff = 0.5;
    _timeSmoothNoiseCoeff = 0.5;

    // Default behavior, computed frequencies are not very accurate
    // (e.g ~6/8Hz accuracy)
    _computeAccurateFreqs = false;
    
    // Optim
    computeAWeights(bufferSize/2, sampleRate);
}

PartialTracker::~PartialTracker()
{
    delete _scale;
}

void
PartialTracker::reset()
{
    _partials.clear();
    _result.clear();
    
    _noiseEnvelope.resize(0);
    _harmonicEnvelope.resize(0);;

    _currentMagns.resize(0);
    _currentPhases.resize(0);
    
    _smoothWinNoise.resize(0);

    _prevNoiseEnvelope.resize(0);

    // For ComputeMusicalNoise()
    _prevNoiseMasks.unfreeze();
    _prevNoiseMasks.clear();
    
    _timeSmoothPrevMagns.resize(0);
    _timeSmoothPrevNoise.resize(0);
}

void
PartialTracker::reset(int bufferSize, float sampleRate)
{
    _bufferSize = bufferSize;
    _sampleRate = sampleRate;
    
    reset();

    // Optim
    computeAWeights(bufferSize/2, sampleRate);
}

void
PartialTracker::setComputeAccurateFreqs(bool flag)
{
    _computeAccurateFreqs = flag;
}

float
PartialTracker::getMinAmpDB()
{
    return MIN_AMP_DB;
}

void
PartialTracker::setThreshold(float threshold)
{
    _threshold = threshold;
}

void
PartialTracker::setData(const vector<float> &magns,
                        const vector<float> &phases)
{
    _currentMagns = magns;
    _currentPhases = phases;
    
    preProcess(&_currentMagns, &_currentPhases);
}

void
PartialTracker::getPreProcessedMagns(vector<float> *magns)
{
    *magns = _currentMagns;
}

void
PartialTracker::detectPartials()
{
    vector<float> &magns0 = _tmpBuf0;
    magns0 = _currentMagns;
    
    vector<Partial> &partials = _tmpPartials0;
    partials.resize(0);
    detectPartials(magns0, _currentPhases, &partials);

    if (_computeAccurateFreqs)
        computeAccurateFreqs(&partials);
    
    suppressZeroFreqPartials(&partials);
    
    // Some operations
    vector<Partial> &prev = _tmpPartials1;
    prev = partials;
    
    gluePartialBarbs(magns0, &partials);
        
    discardFlatPartials(magns0, &partials);
    
    computePeaksHeights(_currentMagns, &partials);
    
    // Threshold
    thresholdPartialsPeakHeight(&partials);
    
    _partials.push_front(partials);
    
    while(_partials.size() > PARTIALS_HISTORY_SIZE)
        _partials.pop_back();
    
    _result = partials;
}

void
PartialTracker::extractNoiseEnvelope()
{    
    extractNoiseEnvelopeSimple();
    
    timeSmoothNoise(&_noiseEnvelope);
}

void
PartialTracker::extractNoiseEnvelopeSimple()
{
    vector<Partial> &partials = _tmpPartials4;
    partials.resize(0);
    
    // Must get the alive partials only,
    // otherwise we would get additional "garbage" partials,
    // that would corrupt the partial rectangle
    // and then compute incorrect noise peaks
    // (the state must be ALIVE, and not _wasAlive !)
    getAlivePartials(&partials);
    
    _harmonicEnvelope = _currentMagns;
    
    // Just in case
    for (int i = 0; i < DETECT_PARTIALS_START_INDEX; i++)
        _harmonicEnvelope.data()[i] = MIN_NORM_AMP;
    
    keepOnlyPartials(partials, &_harmonicEnvelope);
    
    // Compute harmonic envelope
    // (origin signal less noise)
    _noiseEnvelope = _currentMagns;
    
    Utils::substractBuffers(&_noiseEnvelope, _harmonicEnvelope);
    
    Utils::clipMin(&_noiseEnvelope, (float)0.0);
    
    // Avoids interpolation from 0 to the first valid index
    // (could have made an artificial increasing slope in the low freqs)
    for (int i = 0; i < _noiseEnvelope.size(); i++)
    {
        float val = _noiseEnvelope.data()[i];
        if (val > NL_EPS)
            // First value
        {
            int prevIdx = i - 1;
            if (prevIdx > 0)
                _noiseEnvelope.data()[prevIdx] = NL_EPS;
            
            break;
        }
    }
        
    // Works well, but do not remove all musical noise
    processMusicalNoise(&_noiseEnvelope);
    
    // Create an envelope
    Utils::fillMissingValues(&_noiseEnvelope, false, (float)0.0);
}

// Supress musical noise in the raw noise
void
PartialTracker::processMusicalNoise(vector<float> *noise)
{
// Must choose bigger value than 1e-15
// (otherwise the threshold won't work)
#define MUS_NOISE_EPS 1e-8
    
    // Better with an history
    // => suppress only spots that are in zone where partials are erased
    
    // If history is small => remove more spots (but unwanted ones)
    // If history too big => keep some spots that should have been erased
    if (_prevNoiseMasks.size() < HISTORY_SIZE_MUS_NOISE)
    {
        _prevNoiseMasks.push_back(*noise);

        if (_prevNoiseMasks.size() == HISTORY_SIZE_MUS_NOISE)
            _prevNoiseMasks.freeze();
        
        return;
    }
    
    vector<float> &noiseCopy = _tmpBuf2;
    noiseCopy = *noise;
    
    // Search for begin of first isle: values with zero borders
    
    int startIdx = 0;
    while (startIdx < noise->size())
    {
        float val = noise->data()[startIdx];
        if (val < MUS_NOISE_EPS)
        // Zero
            break;
        
        startIdx++;
    }
    
    // Loop to search for isles
    
    while(startIdx < noise->size())
    {
        // Find "isles" in the current noise
        int startIdxIsle = startIdx;
        while (startIdxIsle < noise->size())
        {
            float val = noise->data()[startIdxIsle];
            if (val > MUS_NOISE_EPS)
            // One
                // Start of isle found
                break;
            
            startIdxIsle++;
        }
        
        // Search for isles: values with zero borders
        int endIdxIsle = startIdxIsle;
        while (endIdxIsle < noise->size())
        {
            float val = noise->data()[endIdxIsle];
            if (val < MUS_NOISE_EPS)
            // Zero
            {
                // End of isle found
                
                // Adjust the index to the last zero value
                if (endIdxIsle > startIdxIsle)
                    endIdxIsle--;
                
                break;
            }
            
            endIdxIsle++;
        }
        
        // Check that the prev mask is all zero
        // at in front of the isle
        bool prevMaskZero = true;
        for (int i = 0; i < _prevNoiseMasks.size(); i++)
        {
            const vector<float> &mask = _prevNoiseMasks[i];
            
            for (int j = startIdxIsle; j <= endIdxIsle; j++)
            {
                float prevVal = mask.data()[j];
                if (prevVal > MUS_NOISE_EPS)
                {
                    prevMaskZero = false;
                
                    break;
                }
            }
            
            if (!prevMaskZero)
                break;
        }
        
        if (prevMaskZero)
        // We have a real isle
        {            
            // Earse the isle
            for (int i = startIdxIsle; i <= endIdxIsle; i++)
                noise->data()[i] = 0.0;
        }
        
        startIdx = endIdxIsle + 1;
    }
    
    // Fill the history at the end
    _prevNoiseMasks.push_pop(noiseCopy);
}

void
PartialTracker::keepOnlyPartials(const vector<Partial> &partials,
                                 vector<float> *magns)
{
    vector<float> &result = _tmpBuf4;
    result.resize(magns->size());
    Utils::fillZero(&result);
                   
    for (int i = 0; i < partials.size(); i++)
    {
        const Partial &partial = partials[i];
        
        int minIdx = partial._leftIndex;
        if (minIdx >= magns->size())
            continue;
        
        int maxIdx = partial._rightIndex;
        if (maxIdx >= magns->size())
            continue;
        
        for (int j = minIdx; j <= maxIdx; j++)
        {
            float val = magns->data()[j];
            result.data()[j] = val;
        }
    }
                           
    *magns = result;
}

void
PartialTracker::filterPartials()
{    
    filterPartials(&_result);
}

// For noise envelope extraction, the
// state must be ALIVE, and not _wasAlive
bool
PartialTracker::getAlivePartials(vector<Partial> *partials)
{
    if (_partials.empty())
        return false;
    
    partials->clear();
    
    for (int i = 0; i < _partials[0].size(); i++)
    {
        const Partial &p = _partials[0][i];
        if (p._state == Partial::ALIVE)
        {
            partials->push_back(p);
        }
    }
    
    return true;
}

void
PartialTracker::removeRealDeadPartials(vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials5;
    result.resize(0);
    
    for (int i = 0; i < partials->size(); i++)
    {
        const Partial &p = (*partials)[i];
        if (p._wasAlive)
            result.push_back(p);
    }
    
    *partials = result;
}

void
PartialTracker::getPartials(vector<Partial> *partials)
{
    *partials = _result;
    
    for (int i = 0; i < partials->size(); i++)
        (*partials)[i]._freq = (*partials)[i]._predictedFreq;
    
    removeRealDeadPartials(partials);
}

void
PartialTracker::getPartialsRAW(vector<Partial> *partials)
{
    *partials = _result;
}

void
PartialTracker::clearResult()
{
    _result.clear();
}

void
PartialTracker::getNoiseEnvelope(vector<float> *noiseEnv)
{
    *noiseEnv = _noiseEnvelope;
}

void
PartialTracker::getHarmonicEnvelope(vector<float> *harmoEnv)
{
    *harmoEnv = _harmonicEnvelope;
}

void
PartialTracker::setMaxDetectFreq(float maxFreq)
{
    _maxDetectFreq = maxFreq;
}

// Optimized version (original version removed)
void
PartialTracker::detectPartials(const vector<float> &magns,
                               const vector<float> &phases,
                               vector<Partial> *outPartials)
{
    outPartials->clear();
    
    // prevIndex, currentIndex, nextIndex
    
    // Skip the first ones
    // (to avoid artifacts of very low freq partial)
    //int currentIndex = 0;
    int currentIndex = DETECT_PARTIALS_START_INDEX;
    
    float prevVal = 0.0;
    float nextVal = 0.0;
    float currentVal = 0.0;
    
    int maxDetectIndex = magns.size() - 1;
    
    if (_maxDetectFreq > 0.0)
        maxDetectIndex = _maxDetectFreq*_bufferSize*0.5;
    
    if (maxDetectIndex > magns.size() - 1)
        maxDetectIndex = magns.size() - 1;
    
    while(currentIndex < maxDetectIndex)
    {
        if ((currentVal > prevVal) && (currentVal >= nextVal))
        // Maximum found
        {
            if (currentIndex - 1 >= 0)
            {
                // Take the left and right "feets" of the partial,
                // then the middle.
                // (in order to be more precise)
                
                // Left
                int leftIndex = currentIndex;
                if (leftIndex > 0)
                {
                    float prevLeftVal = magns.data()[leftIndex];
                    while(leftIndex > 0)
                    {
                        leftIndex--;
                        
                        float leftVal = magns.data()[leftIndex];
                        
                        // Stop if we reach 0 or if it goes up again
                        if ((leftVal < MIN_NORM_AMP) || (leftVal > prevLeftVal))
                        {
                            if (leftVal >= prevLeftVal)
                                leftIndex++;
                            
                            // Check bounds
                            if (leftIndex < 0)
                                leftIndex = 0;
                            
                            if (leftIndex > maxDetectIndex)
                                leftIndex = maxDetectIndex;
                            
                            break;
                        }
                        
                        prevLeftVal = leftVal;
                    }
                }
                
                // Right
                int rightIndex = currentIndex;
                
                if (rightIndex <= maxDetectIndex)
                {
                    float prevRightVal = magns.data()[rightIndex];
                    
                    while(rightIndex < maxDetectIndex)
                    {
                        rightIndex++;
                                
                        float rightVal = magns.data()[rightIndex];
                                
                        // Stop if we reach 0 or if it goes up again
                        if ((rightVal < MIN_NORM_AMP) || (rightVal > prevRightVal))
                        {
                            if (rightVal >= prevRightVal)
                                rightIndex--;
                                    
                            // Check bounds
                            if (rightIndex < 0)
                                rightIndex = 0;
                                    
                            if (rightIndex > maxDetectIndex)
                                rightIndex = maxDetectIndex;
                                    
                            break;
                        }
                                
                        prevRightVal = rightVal;
                    }
                }
                
                // Take the max (better than taking the middle)
                int peakIndex = currentIndex;
                
                if ((peakIndex < 0) || (peakIndex > maxDetectIndex))
                // Out of bounds
                    continue;
                
                bool discard = false;
    
                if (!discard)
                    discard = discardInvalidPeaks(magns, peakIndex, leftIndex, rightIndex);
                
                if (!discard)
                {
                    // Create new partial
                    //
                    Partial p;
                    p._leftIndex = leftIndex;
                    p._rightIndex = rightIndex;

                    if (!_computeAccurateFreqs) // Do not recompute 2 times!
                    {                    
                        float peakIndexF =
                            computePeakIndexHalfProminenceAvg(magns,
                                                              peakIndex,
                                                              p._leftIndex,
                                                              p._rightIndex);

                        p._peakIndex = round(peakIndexF);
                        if (p._peakIndex < 0)
                            p._peakIndex = 0;
                    
                        if (p._peakIndex > maxDetectIndex)
                            p._peakIndex = maxDetectIndex;

                        // Remainder: freq is normalized here
                        float peakFreq = peakIndexF/(_bufferSize*0.5);
                        p._freq = peakFreq;
                    
                        // Kalman
                        //
                        // Update the estimate with the first value
                        p._kf.initEstimate(p._freq);
                    
                        // For predicted freq to be freq for the first value
                        p._predictedFreq = p._freq;

                        computePeakMagnPhaseInterp(magns, phases, peakFreq,
                                                   &p._amp, &p._phase);
                    } // end _computeAccurateFreqs
                    
                    outPartials->push_back(p);
                }
                
                // Go just after the right foot of the partial
                currentIndex = rightIndex;
            }
        }
        else
            // No maximum found, continue 1 step
            currentIndex++;
        
        // Update the values
        currentVal = magns.data()[currentIndex];
        
        if (currentIndex - 1 >= 0)
            prevVal = magns.data()[currentIndex - 1];
        
        if (currentIndex + 1 <= maxDetectIndex)
            nextVal = magns.data()[currentIndex + 1];
    }
}

bool
PartialTracker::gluePartialBarbs(const vector<float> &magns,
                                 vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials6;
    result.resize(0);
    bool glued = false;
    
    sort(partials->begin(), partials->end(), Partial::freqLess);
    
    int idx = 0;
    while(idx < partials->size())
    {
        Partial currentPartial = (*partials)[idx];
        
        vector<Partial> &twinPartials = _tmpPartials7;
        twinPartials.resize(0);
        
        twinPartials.push_back(currentPartial);
        
        for (int j = idx + 1; j < partials->size(); j++)
        {
            const Partial &otherPartial = (*partials)[j];
            
            if (otherPartial._leftIndex == currentPartial._rightIndex)
            // This is a twin partial...
            {
                float promCur = computePeakProminence(magns,
                                                      currentPartial._peakIndex,
                                                      currentPartial._leftIndex,
                                                      currentPartial._rightIndex);
                
                float promOther = computePeakProminence(magns,
                                                        otherPartial._peakIndex,
                                                        otherPartial._leftIndex,
                                                        otherPartial._rightIndex);
                
                // Default ratio value
                // If it keeps this value, this is ok, this will be glued
                float ratio = 0.0;
                if (promOther > NL_EPS)
                {
                    ratio = promCur/promOther;
                    if ((ratio > GLUE_BARBS_AMP_RATIO) || (ratio < 1.0/GLUE_BARBS_AMP_RATIO))
                    // ... with a big amp ratio
                    {
                        // Check that the barb is "in the middle" of a side of the main partial
                        // (in height)
                        bool inTheMiddle = false;
                        bool onTheSide = false;
                        if (promCur < promOther)
                        {
                            float hf = computePeakHigherFoot(magns,
                                                             currentPartial._leftIndex,
                                                             currentPartial._rightIndex);

                            
                            float lf = computePeakLowerFoot(magns,
                                                            otherPartial._leftIndex,
                                                            otherPartial._rightIndex);
                            
                            if ((hf > lf) && (hf < otherPartial._amp))
                                inTheMiddle = true;
                            
                            // Check that the barb is on the right side
                            float otherLeftFoot = magns.data()[otherPartial._leftIndex];
                            float otherRightFoot = magns.data()[otherPartial._rightIndex];
                            if (otherLeftFoot > otherRightFoot)
                                onTheSide = true;
                            
                        }
                        else
                        {
                            float hf =
                                computePeakHigherFoot(magns,
                                                      otherPartial._leftIndex,
                                                      otherPartial._rightIndex);
                            
                            
                            float lf =
                                computePeakLowerFoot(magns,
                                                     currentPartial._leftIndex,
                                                     currentPartial._rightIndex);
                            
                            if ((hf > lf) && (hf < currentPartial._amp))
                                inTheMiddle = true;
                            
                            // Check that the barb is on the right side
                            float curLeftFoot =
                                magns.data()[currentPartial._leftIndex];
                            float curRightFoot =
                                magns.data()[currentPartial._rightIndex];
                            if (curLeftFoot < curRightFoot)
                                onTheSide = true;
                        }
                            
                        if (inTheMiddle && onTheSide)
                            // This tween partial is a barb ! 
                            twinPartials.push_back(otherPartial);
                    }
                }
            }
        }
        
        // Glue ?
        
        if (twinPartials.size() > 1)
        {
            glued = true;
            
            // Compute glued partial
            int leftIndex = twinPartials[0]._leftIndex;
            int rightIndex = twinPartials[twinPartials.size() - 1]._rightIndex;
            
            float peakIndex = computePeakIndexAvg(magns, leftIndex, rightIndex);
            
            // For peak amp, take max amp
            float maxAmp = -NL_INF;
            for (int k = 0; k < twinPartials.size(); k++)
            {
                float amp = twinPartials[k]._amp;
                if (amp > maxAmp)
                    maxAmp = amp;
            }
            
            Partial res;
            res._leftIndex = leftIndex;
            res._rightIndex = rightIndex;
            
            // Artificial peak
            res._peakIndex = peakIndex;
            
            float peakFreq = peakIndex/(_bufferSize*0.5);
            res._freq = peakFreq;
            res._amp = maxAmp;
            
            // Kalman
            res._kf.initEstimate(res._freq);
            res._predictedFreq = res._freq;
            
            // Do not set _phase for now
            
            result.push_back(res);
        }
        else
            // Not twin, simply add the partial
            result.push_back(twinPartials[0]);
        
        // 1 or more
        idx += twinPartials.size();
    }
    
    *partials = result;
    
    return glued;
}

bool
PartialTracker::discardFlatPartial(const vector<float> &magns,
                                   int peakIndex, int leftIndex, int rightIndex)
{
    float amp = magns.data()[peakIndex];
    
    float binDiff = rightIndex - leftIndex;
    
    float coeff = binDiff/amp;
    
    bool result = (coeff > DISCARD_FLAT_PARTIAL_COEFF);
    
    return result;
}

void
PartialTracker::discardFlatPartials(const vector<float> &magns,
                                    vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials8;
    result.resize(0);
    
    for (int i = 0; i < partials->size(); i++)
    {
        const Partial &partial = (*partials)[i];
            
        bool discard = discardFlatPartial(magns,
                                          partial._peakIndex,
                                          partial._leftIndex,
                                          partial._rightIndex);
        
        if (!discard)
            result.push_back(partial);
    }
    
    *partials = result;
}

bool
PartialTracker::discardInvalidPeaks(const vector<float> &magns,
                                    int peakIndex, int leftIndex, int rightIndex)
{
    float peakAmp = magns.data()[peakIndex];
    float leftAmp = magns.data()[leftIndex];
    float rightAmp = magns.data()[rightIndex];
    
    if ((peakAmp > leftAmp) && (peakAmp > rightAmp))
        // Correct, do not discard
        return false;
    
    return true;
}

void
PartialTracker::suppressZeroFreqPartials(vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials9;
    result.resize(0);
    
    for (int i = 0; i < partials->size(); i++)
    {
        const Partial &partial = (*partials)[i];
        
        float peakFreq = partial._freq;
        
        // Zero frequency (because of very small magn) ?
        bool discard = false;
        if (peakFreq < NL_EPS)
            discard = true;
        
        if (!discard)
            result.push_back(partial);
    }
    
    *partials = result;
}

void
PartialTracker::thresholdPartialsPeakHeight(vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials10;
    result.resize(0);
    
    for (int i = 0; i < partials->size(); i++)
    {
        const Partial &partial = (*partials)[i];
        
        float height = partial._peakHeight;
        
        // Just in case
        if (height < 0.0)
            height = 0.0;
        
        // Threshold
        //
        
        int binNum = partial._freq*_bufferSize*0.5;
        float thrsNorm = getThreshold(binNum);
        
        if (height >= thrsNorm)
             result.push_back(partial);
    }
    
    *partials = result;
}

// Prominence
float
PartialTracker::computePeakProminence(const vector<float> &magns,
                                      int peakIndex, int leftIndex, int rightIndex)
{
    // Compute prominence
    // See: https://www.mathworks.com/help/signal/ref/findpeaks.html
    float maxFootAmp = magns.data()[leftIndex];
    if (magns.data()[rightIndex] > maxFootAmp)
        maxFootAmp = magns.data()[rightIndex];
    
    float peakAmp = magns.data()[peakIndex];
    
    float prominence = peakAmp - maxFootAmp;
    
    return prominence;
}

// Parabola peak center detection
// Works well (but I prefer my method) 
//
// See: http://eprints.maynoothuniversity.ie/4523/1/thesis.pdf (p32)
//
// and: https://ccrma.stanford.edu/~jos/parshl/Peak_Detection_Steps_3.html#sec:peakdet
//
float
PartialTracker::computePeakIndexParabola(const vector<float> &magns,
                                         int peakIndex)
{
    if ((peakIndex - 1 < 0) || (peakIndex + 1 >= magns.size()))
        return peakIndex;
    
    // magns are in DBn, no need to convert
    float alpha = magns.data()[peakIndex - 1];
    float beta = magns.data()[peakIndex];
    float gamma = magns.data()[peakIndex + 1];

    // Will avoid wrong negative result
    if ((beta < alpha) || (beta < gamma))
        return peakIndex;
    
    // Center
    float denom = (alpha - 2.0*beta + gamma);
    if (fabs(denom) < NL_EPS)
        return peakIndex;
    
    float c = 0.5*((alpha - gamma)/denom);
    
    float result = peakIndex + c;
    
    return result;
}

float
PartialTracker::
computePeakIndexHalfProminenceAvg(const vector<float> &magns,
                                  int peakIndex, int leftIndex, int rightIndex)
{
    // First step: find float indices corresponding to the half prominence
    // Find float indices, and intermediate interpolated magns, for more accuracy
    float prominence =
        computePeakProminence(magns, peakIndex, leftIndex, rightIndex);

    // Half-prominence threshold
    float thrs = magns.data()[peakIndex] - prominence*0.5;
    
    // Left and right float points
    float LP[2];
    LP[0] = leftIndex;
    LP[1] = magns.data()[leftIndex];
    
    float RP[2];
    RP[0] = rightIndex;
    RP[1] = magns.data()[rightIndex];
    
    // Left
    while(LP[0] < peakIndex)
    {
        if (magns.data()[(int)LP[0] + 1] > thrs)
        {
            float m0 = magns.data()[(int)LP[0]];
            float m1 = magns.data()[(int)LP[0] + 1];
            
            float t = (thrs - m0)/(m1 - m0);
            
            LP[0] = LP[0] + t;
            LP[1] = m0 + t*(m1 - m0);
            
            break;
        }
        
        LP[0]++;
    }
    
    // Right
    while(RP[0] > peakIndex)
    {
        if (magns.data()[(int)RP[0] - 1] > thrs)
        {
            float m0 = magns.data()[(int)RP[0]];
            float m1 = magns.data()[(int)RP[0] - 1];
            
            float t = (thrs - m0)/(m1 - m0);
            
            RP[0] = RP[0] - t;
            RP[1] = m0 + t*(m1 - m0);
            
            break;
        }
        
        RP[0]--;
    }
    
    // Second step: compute the result float peak index (weighted avg)
    // Separate first and last float indices from the loop
    float sumMagns = 0.0;
    float sumIndices = 0.0;
    
    // First float point
    sumMagns += LP[1];
    sumIndices += LP[0]*LP[1];
    
    // Middle points
    for (int i = (int)(LP[0] + 1); i <= (int)RP[0]; i++)
    {
        float m = magns.data()[i];
        
        sumMagns += m;
        sumIndices += i*m;
    }
    
    // Last float point
    sumMagns += RP[1];
    sumIndices += RP[0]*RP[1];
    
    if (sumMagns < NL_EPS)
        return 0.0;
    
    // Result
    float indexF = sumIndices/sumMagns;
    
    return indexF;
}

// Inverse of prominence
float
PartialTracker::computePeakHeight(const vector<float> &magns,
                                  int peakIndex, int leftIndex, int rightIndex)
{
    // Compute height
    // See: https://www.mathworks.com/help/signal/ref/findpeaks.html
    float minFootAmp = magns.data()[leftIndex];
    if (magns.data()[rightIndex] < minFootAmp)
        minFootAmp = magns.data()[rightIndex];
    
    float peakAmp = magns.data()[peakIndex];
    
    float height = peakAmp - minFootAmp;
    
    return height;
}

// Compute difference in amp, then convert back to Db
float
PartialTracker::computePeakHeightDb(const vector<float> &magns,
                                    int peakIndex, int leftIndex, int rightIndex,
                                    const Partial &partial)
{
    // Compute height
    // See: https://www.mathworks.com/help/signal/ref/findpeaks.html
    
    // Get in Db
    float minFoot = magns.data()[leftIndex];
    if (magns.data()[rightIndex] < minFoot)
        minFoot = magns.data()[rightIndex];
    
    float peak = partial._amp;
    
    // Compute height
    float height = peak - minFoot;
    
    return height;
}

float
PartialTracker::computePeakHigherFoot(const vector<float> &magns,
                                      int leftIndex, int rightIndex)
{
    float leftVal = magns.data()[leftIndex];
    float rightVal = magns.data()[rightIndex];
    
    if (leftVal > rightVal)
        return leftVal;
    else
        return rightVal;
}

float
PartialTracker::computePeakLowerFoot(const vector<float> &magns,
                                     int leftIndex, int rightIndex)
{
    float leftVal = magns.data()[leftIndex];
    float rightVal = magns.data()[rightIndex];
    
    if (leftVal < rightVal)
        return leftVal;
    else
        return rightVal;
}

void
PartialTracker::computePeaksHeights(const vector<float> &magns,
                                    vector<Partial> *partials)
{
    for (int i = 0; i < partials->size(); i++)
    {
        Partial &partial = (*partials)[i];
        
        float height = computePeakHeight(magns,
                                         partial._peakIndex,
                                         partial._leftIndex,
                                         partial._rightIndex);
        
        partial._peakHeight = height;
    }
}

void
PartialTracker::suppressBarbs(vector<Partial> *partials)
{
#define HEIGHT_COEFF 2.0
#define WIDTH_COEFF 1.0
    
    vector<Partial> &result = _tmpPartials11;
    result.resize(0);
    
    for (int i = 0; i < partials->size(); i++)
    {
        const Partial &partial = (*partials)[i];
    
        // Check if the partial is a barb
        bool isBarb = false;
        for (int j = 0; j < partials->size(); j++)
        {
            const Partial &other = (*partials)[j];
            
            if (other._amp < partial._amp*HEIGHT_COEFF)
                // Amplitudes of tested partial is not small enough
                // compared to the current partial
                // => Not a candidate for barb
                continue;
            
            int center = other._peakIndex;
            int size = other._rightIndex - other._leftIndex;
            
            if ((partial._peakIndex > center - size*WIDTH_COEFF) &&
                (partial._peakIndex < center + size*WIDTH_COEFF))
            {
                // Tested partial peak is "inside" a margin around
                // the current partial.
                // And its amplitude is small compared to the current partial
                // => this is a barb !
                isBarb = true;
                
                break;
            }
        }
        
        // It is not a barb
        if (!isBarb)
            result.push_back(partial);
    }
    
    *partials = result;
}

// Filter
void
PartialTracker::filterPartials(vector<Partial> *result)
{
    result->clear();
    
    if (_partials.empty())
        return;
    
    if (_partials.size() == 1)
        // Assigne ids to the first series of partials
    {
        for (int j = 0; j < _partials[0].size(); j++)
        {
            Partial &currentPartial = _partials[0][j];
            currentPartial.genNewId();
        }
        
        // Not enough partials to filter, need 2 series
        return;
    }
    
    if (_partials.size() < 2)
        return;
    
    const vector<Partial> &prevPartials = _partials[1];
    vector<Partial> &currentPartials = _tmpPartials12;
    currentPartials = _partials[0];
    
    // Partials that was not associated at the end
    vector<Partial> &remainingPartials = _tmpPartials13;
    remainingPartials.resize(0);
    
    associatePartialsPARSHL(prevPartials, &currentPartials, &remainingPartials);
    
    // Add the new zombie and dead partials
    for (int i = 0; i < prevPartials.size(); i++)
    {
        const Partial &prevPartial = prevPartials[i];

        bool found = false;
        for (int j = 0; j < currentPartials.size(); j++)
        {
            const Partial &currentPartial = currentPartials[j];
            
            if (currentPartial._id == prevPartial._id)
            {
                found = true;
                
                break;
            }
        }

        if (!found)
        {
            if (prevPartial._state == Partial::ALIVE)
            {
                // We set zombie for 1 frame only
                Partial newPartial = prevPartial;
                newPartial._state = Partial::ZOMBIE;
                newPartial._zombieAge = 0;
                
                // Kalman:
                // Also extrapolate the zombies
                newPartial._predictedFreq =
                    newPartial._kf.updateEstimate(newPartial._freq);
                
                currentPartials.push_back(newPartial);
            }
            else if (prevPartial._state == Partial::ZOMBIE)
            {
                Partial newPartial = prevPartial;
                
                newPartial._zombieAge++;
                if (newPartial._zombieAge >= MAX_ZOMBIE_AGE)
                    newPartial._state = Partial::DEAD;
  
                // Kalman
                // Also extrapolate the zombies
                newPartial._predictedFreq =
                    newPartial._kf.updateEstimate(newPartial._freq);

                currentPartials.push_back(newPartial);
            }
            
            // If DEAD, do not add, forget it
        }
    }
    
    // Get the result here
    // So we get the partials that are well tracked over time
    *result = currentPartials;
    
    // At the end, there remains the partial that have not been matched
    //
    // Add them at to the history for next time
    //
    for (int i = 0; i < remainingPartials.size(); i++)
    {
        Partial p = remainingPartials[i];
        
        p.genNewId();
        
        currentPartials.push_back(p);
    }
    
    // Then sort the new partials by frequency
    sort(currentPartials.begin(), currentPartials.end(), Partial::freqLess);
    
    //
    // Update: add the partials to the history
    // (except the dead ones)
    _partials[0].clear();
    for (int i = 0; i < currentPartials.size(); i++)
    {
        const Partial &currentPartial = currentPartials[i];
        
        _partials[0].push_back(currentPartial);
    }
}

// Better than "Simple" => do not make jumps between bins
float
PartialTracker::computePeakIndexAvg(const vector<float> &magns,
                                    int leftIndex, int rightIndex)
{
    // Pow coeff, to select preferably the high amp values
    // With 2.0, makes smoother freq change
    // With 3.0, make just a little smoother than 2.0
#define COEFF 3.0
    
    float sumIndex = 0.0;
    float sumMagns = 0.0;
    
    for (int i = leftIndex; i <= rightIndex; i++)
    {
        float magn = magns.data()[i];
        
        magn = std::pow(magn, COEFF);
        
        sumIndex += i*magn;
        sumMagns += magn;
    }
    
    if (sumMagns < NL_EPS)
        return 0.0;
    
    float result = sumIndex/sumMagns;
    
    return result;
}

float
PartialTracker::computePeakIndexAvgSimple(const vector<float> &magns,
                                          int leftIndex, int rightIndex)
{
    float sumIndex = 0.0;
    float sumMagns = 0.0;
    for (int i = leftIndex; i <= rightIndex; i++)
    {
        float magn = magns.data()[i];
        
        sumIndex += i*magn;
        sumMagns += magn;
    }
    
    if (sumMagns < NL_EPS)
        return 0.0;
    
    float result = sumIndex/sumMagns;
    
    return result;
}

void
PartialTracker::computePeakMagnPhaseInterp(const vector<float> &magns,
                                           const vector<float> &uwPhases,
                                           float peakFreq,
                                           float *peakAmp, float *peakPhase)
{
    // Phases are unwrapped here
    
    float bin = peakFreq*_bufferSize*0.5;
    
    int prevBin = (int)bin;
    int nextBin = (int)bin + 1;
    
    if (nextBin >= magns.size())
    {
        *peakAmp = magns.data()[prevBin];
        *peakPhase = uwPhases.data()[prevBin];
        
        return;
    }
    
    // Interpolate
    float t = bin - prevBin;
    
    *peakAmp = (1.0 - t)*magns.data()[prevBin] + t*magns.data()[nextBin];
    *peakPhase = (1.0 - t)*uwPhases.data()[prevBin] + t*uwPhases.data()[nextBin];
}

int
PartialTracker::findPartialById(const vector<PartialTracker::Partial> &partials, int idx)
{
    for (int i = 0; i < partials.size(); i++)
    {
        const Partial &partial = partials[i];
        
        if (partial._id == idx)
            return i;
    }
    
    return -1;
}

// Use method similar to SAS
void
PartialTracker::
associatePartials(const vector<PartialTracker::Partial> &prevPartials,
                  vector<PartialTracker::Partial> *currentPartials,
                  vector<PartialTracker::Partial> *remainingPartials)
{
    // Sort current partials and prev partials by decreasing amplitude
    vector<Partial> &currentPartialsSort = _tmpPartials14;
    currentPartialsSort = *currentPartials;
    sort(currentPartialsSort.begin(), currentPartialsSort.end(), Partial::ampLess);
    reverse(currentPartialsSort.begin(), currentPartialsSort.end());
    
    vector<Partial> &prevPartialsSort = _tmpPartials15;
    prevPartialsSort = prevPartials;
    
    sort(prevPartialsSort.begin(), prevPartialsSort.end(), Partial::ampLess);
    reverse(prevPartialsSort.begin(), prevPartialsSort.end());
 
    // Associate
    
    // Associated partials
    vector<Partial> &currentPartialsAssoc = _tmpPartials16;
    currentPartialsAssoc.resize(0);
    
    for (int i = 0; i < prevPartialsSort.size(); i++)
    {
        const Partial &prevPartial = prevPartialsSort[i];
        
        for (int j = 0; j < currentPartialsSort.size(); j++)
        {
            Partial &currentPartial = currentPartialsSort[j];
            
            if (currentPartial._id != -1)
                // Already assigned
                continue;
            
            float diffFreq = std::fabs(prevPartial._freq - currentPartial._freq);
            
            int binNum = currentPartial._freq*_bufferSize*0.5;
            float diffCoeff = getDeltaFreqCoeff(binNum);
            if (diffFreq < DELTA_FREQ_ASSOC*diffCoeff)
            // Associated !
            {
                currentPartial._id = prevPartial._id;
                currentPartial._state = Partial::ALIVE;
                currentPartial._wasAlive = true;
                
                currentPartial._age = prevPartial._age + 1;
            
                // Kalman
                currentPartial._kf = prevPartial._kf;
                currentPartial._predictedFreq =
                    currentPartial._kf.updateEstimate(currentPartial._freq);
                
                currentPartialsAssoc.push_back(currentPartial);
                
                // We have associated to the prev partial
                // We are done!
                // Stop the search here.
                break;
            }
        }
    }
    
    sort(currentPartialsAssoc.begin(), currentPartialsAssoc.end(), Partial::idLess);
     *currentPartials = currentPartialsAssoc;
    
    // Add the remaining partials
    remainingPartials->clear();
    for (int i = 0; i < currentPartialsSort.size(); i++)
    {
        const Partial &p = currentPartialsSort[i];
        if (p._id == -1)
            remainingPartials->push_back(p);
    }
}

// Use PARSHL method
void
PartialTracker::
associatePartialsPARSHL(const vector<PartialTracker::Partial> &prevPartials,
                        vector<PartialTracker::Partial> *currentPartials,
                        vector<PartialTracker::Partial> *remainingPartials)
{
    // Sort current partials and prev partials by increasing frequency
    sort(currentPartials->begin(), currentPartials->end(), Partial::freqLess);
    
    vector<PartialTracker::Partial> &prevPartials0 = _tmpPartials17;
    prevPartials0 = prevPartials;
    sort(prevPartials0.begin(), prevPartials0.end(), Partial::freqLess);
    
    // Associated partials
    bool stopFlag = true;
    do {
        stopFlag = true;
        
        for (int i = 0; i < prevPartials0.size(); i++)
        {
            const Partial &prevPartial = prevPartials0[i];
            for (int j = 0; j < currentPartials->size(); j++)
            {
                Partial &currentPartial = (*currentPartials)[j];
                if (currentPartial._id != -1)
                    // Already associated, nothing to do on this step!
                    continue;
                
                float diffFreq =
                    fabs(prevPartial._freq - currentPartial._freq);

                int binNum = currentPartial._freq*_bufferSize*0.5;
                float diffCoeff = getDeltaFreqCoeff(binNum);
            
                if (diffFreq < DELTA_FREQ_ASSOC*diffCoeff)
                    // Associate!
                {
                    int otherIdx =
                        findPartialById(*currentPartials, (int)prevPartial._id);
                    
                    if (otherIdx == -1)
                        // This partial is not yet associated
                        // => No fight
                    {
                        currentPartial._id = prevPartial._id;
                        currentPartial._age = prevPartial._age;
                        currentPartial._kf = prevPartial._kf;
                        
                        stopFlag = false;
                    }
                    else // Fight!
                    {
                        Partial &otherPartial = (*currentPartials)[otherIdx];
                        
                        float otherDiffFreq =
                            fabs(prevPartial._freq - otherPartial._freq);
                        
                        if (diffFreq < otherDiffFreq)
                        // Current partial won
                        {
                            currentPartial._id = prevPartial._id;
                            currentPartial._age = prevPartial._age;
                            currentPartial._kf = prevPartial._kf; //
                            
                            // Detach the other
                            otherPartial._id = -1;
                            
                            stopFlag = false;
                        }
                        else
                        // Other partial won
                        {
                            // Just keep it like it is!
                        }
                    }
                }
            }
        }
    } while (!stopFlag);
    
    
    // Update partials
    vector<PartialTracker::Partial> &newPartials = _tmpPartials18;
    newPartials.resize(0);
    
    for (int j = 0; j < currentPartials->size(); j++)
    {
        Partial &currentPartial = (*currentPartials)[j];
        
        if (currentPartial._id != -1)
        {
            currentPartial._state = Partial::ALIVE;
            currentPartial._wasAlive = true;
    
            // Increment age
            currentPartial._age = currentPartial._age + 1;
            currentPartial._predictedFreq =
                    currentPartial._kf.updateEstimate(currentPartial._freq);
    
            newPartials.push_back(currentPartial);
        }
    }
    
    // Add the remaining partials
    remainingPartials->clear();
    for (int i = 0; i < currentPartials->size(); i++)
    {
        const Partial &p = (*currentPartials)[i];
        if (p._id == -1)
            remainingPartials->push_back(p);
    }
    
    // Update current partials
    *currentPartials = newPartials;
}

float
PartialTracker::getThreshold(int binNum)
{
    float thrsNorm = -(MIN_AMP_DB - _threshold)/(-MIN_AMP_DB);
    
    return thrsNorm;
}

float
PartialTracker::getDeltaFreqCoeff(int binNum)
{
#define END_COEFF 0.25
    
    float t = ((float)binNum)/(_bufferSize*0.5);
    float diffCoeff = 1.0 - (1.0 - END_COEFF)*t;
    
    return diffCoeff;
}

void
PartialTracker::preProcessDataX(vector<float> *data)
{
    // Use FilterBank internally to avoid stairs effect
    vector<float> &scaledData = _tmpBuf8;
    Scale::FilterBankType type = _scale->typeToFilterBankType(_xScale);
    _scale->applyScaleFilterBank(type, &scaledData, *data,
                                 _sampleRate, data->size());
    *data = scaledData;
}

void
PartialTracker::preProcessDataY(vector<float> *data)
{
    // Y
    _scale->applyScaleForEach(_yScale, data, (float)MIN_AMP_DB, (float)0.0);
    
    // Better tracking on high frequencies with this!
    preProcessAWeighting(data, true);
}

void
PartialTracker::preProcessDataXY(vector<float> *data)
{
    // Process Y first
    preProcessDataY(data);
    preProcessDataX(data);
}
    
// Unwrap phase before converting to mel
void
PartialTracker::preProcess(vector<float> *magns, vector<float> *phases)
{
    // Smooth only magns
    preProcessTimeSmooth(magns);

    // Use time smooth on raw magns too
    // (time smoothed, but linearly scaled)
    _linearMagns = *magns;
    preProcessDataY(&_linearMagns); // We want raw data in dB (just keep linear on x)
        
    preProcessDataXY(magns);
    
    Utils::unwrapPhases(phases);

    // Phases
    vector<float> &scaledPhases = _tmpBuf9;
    Scale::FilterBankType type = _scale->typeToFilterBankType(_xScale);
    _scale->applyScaleFilterBank(type, &scaledPhases, *phases,
                                 _sampleRate, phases->size());
    *phases = scaledPhases;
}

void
PartialTracker::setTimeSmoothCoeff(float coeff)
{
    _timeSmoothCoeff = coeff;
}

void
PartialTracker::setTimeSmoothNoiseCoeff(float coeff)
{
    _timeSmoothNoiseCoeff = coeff;
}

// Time smooth
void
PartialTracker::preProcessTimeSmooth(vector<float> *magns)
{
    if (_timeSmoothPrevMagns.size() == 0)
    {
        _timeSmoothPrevMagns = *magns;
        
        return;
    }
    
    for (int i = 0; i < magns->size(); i++)
    {
        float val = magns->data()[i];
        float prevVal = _timeSmoothPrevMagns.data()[i];
        
        float newVal = (1.0 - _timeSmoothCoeff)*val + _timeSmoothCoeff*prevVal;
        
        magns->data()[i] = newVal;
    }
    
    _timeSmoothPrevMagns = *magns;
}

// Time smooth noise
void
PartialTracker::timeSmoothNoise(vector<float> *noise)
{
    if (_timeSmoothPrevNoise.size() == 0)
    {
        _timeSmoothPrevNoise = *noise;
        
        return;
    }
    
    for (int i = 0; i < noise->size(); i++)
    {
        float val = noise->data()[i];
        float prevVal = _timeSmoothPrevNoise.data()[i];
        
        float newVal = (1.0 - _timeSmoothNoiseCoeff)*val + _timeSmoothNoiseCoeff*prevVal;
        
        noise->data()[i] = newVal;
    }
    
    _timeSmoothPrevNoise = *noise;
}

void
PartialTracker::denormPartials(vector<PartialTracker::Partial> *partials)
{
    float hzPerBin =  _sampleRate/_bufferSize;
    
    for (int i = 0; i < partials->size(); i++)
    {
        PartialTracker::Partial &partial = (*partials)[i];
        
        // Reverse Mel
        float freq = partial._freq;
        freq = _scale->applyScale(_xScaleInv, freq, (float)0.0,
                                  (float)(_sampleRate*0.5));
        partial._freq = freq;
        
        // Convert to real freqs
        partial._freq *= _sampleRate*0.5;
        
        // Reverse AWeighting
        int binNum = partial._freq/hzPerBin;
        partial._amp = processAWeighting(binNum, _bufferSize*0.5,
                                         partial._amp, false);
    
        // Y
        partial._amp = _scale->applyScale(_yScaleInv, partial._amp,
                                          (float)MIN_AMP_DB, (float)0.0);

        partial._leftIndex = denormBinIndex(partial._leftIndex);
        partial._peakIndex = denormBinIndex(partial._peakIndex);
        partial._rightIndex = denormBinIndex(partial._rightIndex);
    }
}

void
PartialTracker::denormData(vector<float> *data)
{
    // Use FilterBank internally to avoid stairs effect
    vector<float> &scaledData = _tmpBuf7;
    Scale::FilterBankType type = _scale->typeToFilterBankType(_xScale);
    _scale->applyScaleFilterBankInv(type, &scaledData, *data,
                                    _sampleRate, data->size());
    *data = scaledData;
    
    // A-Weighting
    preProcessAWeighting(data, false);
    
    // Y
    _scale->applyScaleForEach(_yScaleInv, data, (float)MIN_AMP_DB, (float)0.0);
}

void
PartialTracker::partialsAmpToAmpDB(vector<PartialTracker::Partial> *partials)
{
    for (int i = 0; i < partials->size(); i++)
    {
        PartialTracker::Partial &partial = (*partials)[i];
        
        partial._ampDB = Utils::ampToDB(partial._amp);
    }
}

void
PartialTracker::preProcessAWeighting(vector<float> *magns, bool reverse)
{
    // Input magns are in normalized dB
    
    vector<float> &weights = _tmpBuf6;
    weights = _aWeights;
    
    float hzPerBin = 0.5*_sampleRate/magns->size();
    
    // W-Weighting property: 0dB at 1000Hz!
    float zeroDbFreq = 1000.0;
    int zeroDbBin = zeroDbFreq/hzPerBin;
    
    for (int i = zeroDbBin; i < magns->size(); i++)
    {
        float a = weights.data()[i];
        
        float normDbMagn = magns->data()[i];
        float dbMagn = (1.0 - normDbMagn)*MIN_AMP_DB;
        
        if (reverse)
            dbMagn -= a;
        else
            dbMagn += a;
        
        normDbMagn = 1.0 - dbMagn/MIN_AMP_DB;
        
        if (normDbMagn < 0.0)
            normDbMagn = 0.0;
        if (normDbMagn > 1.0)
            normDbMagn = 1.0;
        
        magns->data()[i] = normDbMagn;
    }
}

float
PartialTracker::processAWeighting(int binNum, int numBins,
                                  float magn, bool reverse)
{
    // Input magn is in normalized dB
    
    float hzPerBin = 0.5*_sampleRate/numBins;
    
    // W-Weighting property: 0dB at 1000Hz!
    float zeroDbFreq = 1000.0;
    int zeroDbBin = zeroDbFreq/hzPerBin;
    
    if (binNum <= zeroDbBin)
        // Do nothing
        return magn;
    
    //float a = AWeighting::ComputeAWeight(binNum, numBins, _sampleRate);
    float a = _aWeights.data()[binNum];
    
    float normDbMagn = magn;
    float dbMagn = (1.0 - normDbMagn)*MIN_AMP_DB;
        
    if (reverse)
        dbMagn -= a;
    else
        dbMagn += a;
        
    normDbMagn = 1.0 - dbMagn/MIN_AMP_DB;
        
    if (normDbMagn < 0.0)
        normDbMagn = 0.0;
    if (normDbMagn > 1.0)
        normDbMagn = 1.0;
        
    return normDbMagn;
}

// Optim: pre-compute a weights
void
PartialTracker::computeAWeights(int numBins, float sampleRate)
{
    AWeighting::computeAWeights(&_aWeights, numBins, sampleRate);
}

int
PartialTracker::denormBinIndex(int idx)
{
    float freq = ((float)idx)/(_bufferSize*0.5);
    freq = _scale->applyScale(_xScaleInv, freq, (float)0.0,
                              (float)(_sampleRate*0.5));

    float res = freq*(_bufferSize*0.5);

    int resi = round(res);
    if (resi < 0)
        resi = 0;
    if (resi > _bufferSize*0.5 - 1)
        resi = _bufferSize*0.5 - 1;

    return resi;
}

void
PartialTracker::computeAccurateFreqs(vector<Partial> *partials)
{    
    // The most accurate frequencies are acheived using linear scale on x,
    // db scale on y, and parabola peak finding,
    // (and with and offset of 0.5 to get centered on bins) <- mistake ?

    // With this method, we get an accuracy of less than 1Hz!
    
    // Compute the frequencies using this most accurate method
    for (int i = 0; i < partials->size(); i++)
    {
        Partial &p = (*partials)[i];

        // First, find the left and right indices, scaled to linear
        float leftIndexF = ((float)p._leftIndex)/(_bufferSize*0.5);
        float rightIndexF = ((float)p._rightIndex)/(_bufferSize*0.5);

        leftIndexF = _scale->applyScale(_xScaleInv, leftIndexF, (float)0.0,
                                        (float)(_sampleRate*0.5));
        rightIndexF = _scale->applyScale(_xScaleInv, rightIndexF, (float)0.0,
                                         (float)(_sampleRate*0.5));

        float leftIndex0 = leftIndexF*(_bufferSize*0.5);
        float rightIndex0 = rightIndexF*(_bufferSize*0.5);

        // Necessary to round(), otherwise we have the risk to have several peaks in
        // the range [leftIndex, leftIndex] (due to inaccurate L/R bounds)
        int leftIndex = round(leftIndex0);
        int rightIndex = round(rightIndex0);
        
        // Then find the integer peak index, still in linear scale
        // Use the raw magns we previously kept (possibly time smoothed) 
        int peakIndex = Utils::findMaxIndex(_linearMagns, leftIndex, rightIndex);

        // Won't work well with peaks with almost flat top ?
        //
        // Make smooth partials change
        // But makes wobble in the sound volume
        float peakIndexF = computePeakIndexParabola(_linearMagns, peakIndex);
        
        // NOTE: this seemed to be a mistake, fixed here by a hack
        //
        // Then adjust to be centered on the bins (the magic comes here :)
        // => this way we get the right result
        //peakIndexF = peakIndexF + 0.5;

        // Update the partial peak index (just in case)
        p._peakIndex = round(peakIndexF);
        
        // Get the normalized frequency (linear scale)
        float peakFreq = peakIndexF/(_bufferSize*0.5);
                        
        // Rescale it to the current scale
        peakFreq = _scale->applyScale(_xScale, peakFreq, (float)0.0,
                                      (float)(_sampleRate*0.5));

        // And finally, update the partial
        p._freq = peakFreq;

        // Some updates
        //

        // NOTE: not sure this computation is very exact...
        //
        // Update the partial peak index (just in case)
        float newPeakIndex = peakFreq*(_bufferSize*0.5);
        p._peakIndex = round(newPeakIndex);
        if (p._peakIndex < 0)
            p._peakIndex = 0;
        
        int maxDetectIndex = _currentMagns.size();
        if (_maxDetectFreq > 0.0)
            maxDetectIndex = _maxDetectFreq*(_bufferSize*0.5);
        if (maxDetectIndex > _currentMagns.size() - 1)
            maxDetectIndex = _currentMagns.size() - 1;
        if (p._peakIndex > maxDetectIndex)
            p._peakIndex = maxDetectIndex;
    
        // Kalman
        
        // Update the estimate with the first value
        //p._kf.updateEstimate(p._freq);
        p._kf.initEstimate(p._freq);
                    
        // For predicted freq to be freq for the first value
        p._predictedFreq = p._freq;

        // (Re)compute amp more accurately
        computePeakMagnPhaseInterp(_currentMagns, _currentPhases, peakFreq,
                                   &p._amp, &p._phase);
    }
}
