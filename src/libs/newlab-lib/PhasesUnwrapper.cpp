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
#include "PhasesUnwrapper.h"

// Compute a global min and max diff val for phase freq
// Because we don't know how to compute the theorical min and max value.
// NOTE: not perfect, take some time to stabilize
#define FREQ_GLOBAL_MIN_MAX 1

PhasesUnwrapper::PhasesUnwrapper(long historySize)
{
    _historySize = historySize;
    
#if FREQ_GLOBAL_MIN_MAX
    _globalMinDiff = INF;
    _globalMaxDiff = -INF;
#endif
}

PhasesUnwrapper::~PhasesUnwrapper() {}

void
PhasesUnwrapper::reset()
{
    _unwrappedPhasesTime.clear();
    _unwrappedPhasesFreqs.clear();
    
#if FREQ_GLOBAL_MIN_MAX
    _globalMinDiff = INF;
    _globalMaxDiff = -INF;
#endif
}

void
PhasesUnwrapper::setHistorySize(long historySize)
{
    _historySize = historySize;
    
    while(_unwrappedPhasesTime.size() > _historySize)
        _unwrappedPhasesTime.pop_front();
    
    while(_unwrappedPhasesFreqs.size() > _historySize)
        _unwrappedPhasesFreqs.pop_front();
}

void
PhasesUnwrapper::unwrapPhasesFreq(WDL_TypedBuf<float> *phases)
{
    Utils::unwrapPhases(phases, false);
}

void
PhasesUnwrapper::normalizePhasesFreq(WDL_TypedBuf<float> *phases)
{
    if (phases->GetSize() == 0)
        return;
    
    // If we display the phases for bins [1-1024],
    // we see a line with factor 3.14, with very slight changes
    // over this line for each bin
    
    // Compute the phase diffs
    float theoricalPhase = 0.0;
    for (int i = 0; i < phases->size(); i++)
    {
        float phase = phases->data()[i];
        
        float diff = phase - theoricalPhase;
        
        phases->data()[i] = diff;
        
        theoricalPhase += M_PI;
    }
    
#if !FREQ_GLOBAL_MIN_MAX
    // Compute the global extrema (for normalization)
    float minDiffVal = INF;
    float maxDiffVal = -INF;
    for (int i = 0; i < _unwrappedPhasesFreqs.size(); i++)
    {
        const vector<float> &line = _unwrappedPhasesFreqs[i];

        float minimum = Utils::computeMin(line);
        if (minimum < minDiffVal)
            minDiffVal = minimum;
        
        float maximum = Utils::computeMax(line);
        if (maximum > maxDiffVal)
            maxDiffVal = maximum;
    }
#else
    // Take the min and max diff over all the time
    // => avoid jumps
    float minimum = Utils::computeMin(*phases);
    if (minimum < _globalMinDiff)
        _globalMinDiff = minimum;
    
    float maximum = Utils::computeMax(*phases);
    if (maximum > _globalMaxDiff)
        _globalMaxDiff = maximum;

    float minDiffVal = _globalMinDiff;
    float maxDiffVal = _globalMaxDiff;
#endif
    
    // Add to history for next time
    _unwrappedPhasesFreqs.push_back(*phases);
    while(_unwrappedPhasesFreqs.size() > _historySize)
        _unwrappedPhasesFreqs.pop_front();
    
    // Normalize
    Utils::normalize(phases, minDiffVal, maxDiffVal);
}

void
PhasesUnwrapper::computePhasesGradientFreqs(vector<float> *phases)
{
    vector<float> result;
    result.resize(phases->size());
    
    result.data()[0] = 0.0;
    for (int i = 1; i < phases->size(); i++)
    {
        float prevPhase = phases->data()[i - 1];
        float currentPhase = phases->data()[i];
        
        float diff = fabs(currentPhase - prevPhase);
        result.data()[i] = diff;
    }
    
    *phases = result;
}

void
PhasesUnwrapper::normalizePhasesGradientFreqs(vector<float> *phases)
{
    Utils::normalize(phases);
}

void
PhasesUnwrapper::unwrapPhasesTime(vector<float> *phases)
{
    if (_unwrappedPhasesTime.empty())
    {
        // Unwrap the first value
        Utils::unwrapPhases(phases);
        
        _unwrappedPhasesTime.push_back(*phases);
        
        return;
    }
    
    // Prev line
    const vector<float> &prevUnwrapPhases =
        _unwrappedPhasesTime[_unwrappedPhasesTime.size() - 1];
    
    for (int i = 0; i < phases->size(); i++)
    {
        float prevPhase = prevUnwrapPhases.Get()[i];
        Utils::findNextPhase(&prevPhase, (float)0.0);
                
        float phase = phases->data()[i];
        Utils::findNextPhase(&phase, prevPhase);
        phases->data()[i] = phase;
    }
    
    _unwrappedPhasesTime.push_back(*phases);
    while(_unwrappedPhasesTime.size() > _historySize)
        _unwrappedPhasesTime.pop_front();
}

// See: http://kth.diva-portal.org/smash/get/diva2:1381398/FULLTEXT01.pdf
// "Pitch-shifting algorithm design and applications in musicé - THEO ROYER
// (phase unwrapping in time very well explained)
void
PhasesUnwrapper::computeUwPhasesDiffTime(vector<float> *diff,
                                         const vector<float> &phases0,
                                         const vector<float> &phases1,
                                         float sampleRate, int bufferSize,
                                         int overlapping)
{
    diff->resize(phases0.GetSize());

    float hzPerBin = sampleRate/bufferSize;
    // Interval between 2 measurements
    float h = (bufferSize/sampleRate)/overlapping;
        
    for (int i = 0; i < diff->GetSize(); i++)
    {
        float p0 = phases0.data()[i];
        float p1 = phases1.data()[i];

        // Frequency of current bin
        float fk = i*hzPerBin;

        float omegaK = 2.0*M_PI*(bufferSize/overlapping)*
            ((float)i)/bufferSize;
        
        float dp = omegaK + Utils::princarg(p1 - p0 - omegaK);
        
        diff->data()[i] = dp;
    }
}

void
PhasesUnwrapper::normalizePhasesTime(vector<float> *phases)
{
    if (_unwrappedPhasesTime.empty())
    {
        Utils::Normalize(phases);
        
        return;
    }
    
    const vector<float> &oldPhases = _unwrappedPhasesTime[0];
    const vector<float> &newPhases = _unwrappedPhasesTime[_unwrappedPhasesTime.size() - 1];
    
    // we will have a shift in time
    int middle = _unwrappedPhasesTime.size()/2;
    
    if (middle < 0)
        middle = 0;
    const vector<float> &middlePhases = _unwrappedPhasesTime[middle];

    for (int i = 0; i < phases->size(); i++)
    {
        float minimum = oldPhases.data()[i];
        float maximum = newPhases.data()[i];
        
        float phase = middlePhases.data()[i];
        
        phase = Utils::normalize(phase, minimum, maximum);

        phases->data()[i] = phase;
    }
}

void
PhasesUnwrapper::unwrapPhasesTime(const vector<float> &phases0,
                                  vector<float> *phases1)
{
    for (int i = 0; i < phases1->size(); i++)
    {
        float p0 = phases0.data()[i];
                
        float p1 = phases1->data()[i];
        Utils::findNextPhase(&p1, p0);

        phases1->data()[i] = p1;
    }
}

void
PhasesUnwrapper::computePhasesGradientTime(vector<float> *phases)
{
    if (_unwrappedPhasesTime.size() < 2)
    {
        Utils::fillZero(phases);
        
        return;
    }
    
    const vector<float> &currentPhases = *phases;
    const vector<float> &prevPhases =
                    _unwrappedPhasesTime[_unwrappedPhasesTime.size() - 2];
    
    phases->data()[0] = 0.0;
    for (int i = 1; i < currentPhases.GetSize(); i++)
    {
        float prevPhase = prevPhases.Get()[i];
        float currentPhase = currentPhases.Get()[i];
        
        float diff = fabs(currentPhase - prevPhase);
        phases->data()[i] = diff;
    }
}

void
PhasesUnwrapper::normalizePhasesGradientTime(vector<float> *phases)
{
     Utils::normalize(phases);
}
