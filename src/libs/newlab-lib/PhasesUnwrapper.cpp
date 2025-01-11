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

#include <BLUtils.h>
#include <BLUtilsMath.h>
#include <BLUtilsPhases.h>

#include "PhasesUnwrapper.h"

#define INF 1e15

#define NORMALIZE_TIME_PER_BIN 1

// Without this, the value will always be 1 (maximum)
#define TIME_TAKE_MIDDLE_VALUE 1

// Compute difference from the theorical phase
#define FREQ_COMPUTE_DIFF 1

// Compute a global min and max diff val for phase freq
// Because we don't know how to compute the theorical min and max value.
// NOTE: not perfect, take some time to stabilize
#define FREQ_GLOBAL_MIN_MAX 1

PhasesUnwrapper::PhasesUnwrapper(long historySize)
{
    mHistorySize = historySize;
    
#if FREQ_GLOBAL_MIN_MAX
    mGlobalMinDiff = INF;
    mGlobalMaxDiff = -INF;
#endif
}

PhasesUnwrapper::~PhasesUnwrapper() {}

void
PhasesUnwrapper::Reset()
{
    mUnwrappedPhasesTime.clear();
    mUnwrappedPhasesFreqs.clear();
    
#if FREQ_GLOBAL_MIN_MAX
    mGlobalMinDiff = INF;
    mGlobalMaxDiff = -INF;
#endif
}

void
PhasesUnwrapper::SetHistorySize(long historySize)
{
    mHistorySize = historySize;
    
    while(mUnwrappedPhasesTime.size() > mHistorySize)
        mUnwrappedPhasesTime.pop_front();
    
    while(mUnwrappedPhasesFreqs.size() > mHistorySize)
        mUnwrappedPhasesFreqs.pop_front();
}

void
PhasesUnwrapper::UnwrapPhasesFreq(WDL_TypedBuf<BL_FLOAT> *phases)
{
    //
    BLUtilsPhases::UnwrapPhases(phases, false);
}

void
PhasesUnwrapper::NormalizePhasesFreq(WDL_TypedBuf<BL_FLOAT> *phases)
{
    // Old method, very simple, bad
    //BLUtils::Normalize(phases);
    
    if (phases->GetSize() == 0)
        return;
    
#if !FREQ_COMPUTE_DIFF
    BL_FLOAT startVal = phases->Get()[0];
    BLUtils::AddValues(phases, -startVal);
    
    BL_FLOAT coeff = 1.0/(2.0*M_PI);
    
    // IMPORTANT: we potentially increase by Pi at each bin!
    coeff /= phases->GetSize();
    
    BLUtils::MultValues(phases, coeff);
#else
    // If we display the phases for bins [1-1024],
    // we see a line with factor 3.14, with very slight changes
    // over this line for each bin
    
    // Compute the phase diffs
    BL_FLOAT theoricalPhase = 0.0;
    for (int i = 0; i < phases->GetSize(); i++)
    {
        BL_FLOAT phase = phases->Get()[i];
        
        BL_FLOAT diff = phase - theoricalPhase;
        
        // TEST
        //diff = std::fabs(diff);
        
        phases->Get()[i] = diff;
        
        theoricalPhase += M_PI;
    }
    
#if !FREQ_GLOBAL_MIN_MAX
    // BAD: makes jumps in color and y if we don't use fabs on diff
    
    // Compute the global extrema (for normalization)
    BL_FLOAT minDiffVal = INF;
    BL_FLOAT maxDiffVal = -INF;
    for (int i = 0; i < mUnwrappedPhasesFreqs.size(); i++)
    {
        const WDL_TypedBuf<BL_FLOAT> &line = mUnwrappedPhasesFreqs[i];

        BL_FLOAT minimum = BLUtils::ComputeMin(line);
        if (minimum < minDiffVal)
            minDiffVal = minimum;
        
        BL_FLOAT maximum = BLUtils::ComputeMax(line);
        if (maximum > maxDiffVal)
            maxDiffVal = maximum;
    }
#else
    // Good take the min and max diff over all the time
    // => avoid jumps
    // (because we don't know how to compute them mathematically...)
    BL_FLOAT minimum = BLUtils::ComputeMin(*phases);
    if (minimum < mGlobalMinDiff)
        mGlobalMinDiff = minimum;
    
    BL_FLOAT maximum = BLUtils::ComputeMax(*phases);
    if (maximum > mGlobalMaxDiff)
        mGlobalMaxDiff = maximum;

    BL_FLOAT minDiffVal = mGlobalMinDiff;
    BL_FLOAT maxDiffVal = mGlobalMaxDiff;
#endif
    
    // Add to history for next time
    mUnwrappedPhasesFreqs.push_back(*phases);
    while(mUnwrappedPhasesFreqs.size() > mHistorySize)
        mUnwrappedPhasesFreqs.pop_front();
    
    // Normalize
    BLUtils::Normalize(phases, minDiffVal, maxDiffVal);
#endif
}

void
PhasesUnwrapper::ComputePhasesGradientFreqs(WDL_TypedBuf<BL_FLOAT> *phases)
{
    WDL_TypedBuf<BL_FLOAT> result;
    result.Resize(phases->GetSize());
    
    result.Get()[0] = 0.0;
    for (int i = 1; i < phases->GetSize(); i++)
    {
        BL_FLOAT prevPhase = phases->Get()[i - 1];
        BL_FLOAT currentPhase = phases->Get()[i];
        
        BL_FLOAT diff = std::fabs(currentPhase - prevPhase);
        result.Get()[i] = diff;
    }
    
    *phases = result;
}

void
PhasesUnwrapper::NormalizePhasesGradientFreqs(WDL_TypedBuf<BL_FLOAT> *phases)
{
    BLUtils::Normalize(phases);
}

void
PhasesUnwrapper::UnwrapPhasesTime(WDL_TypedBuf<BL_FLOAT> *phases)
{
    if (mUnwrappedPhasesTime.empty())
    {
        // Unwrap the first value
        BLUtilsPhases::UnwrapPhases(phases);
        
        mUnwrappedPhasesTime.push_back(*phases);
        
        return;
    }
    
    // Prev line
    const WDL_TypedBuf<BL_FLOAT> &prevUnwrapPhases =
        mUnwrappedPhasesTime[mUnwrappedPhasesTime.size() - 1];
    
    for (int i = 0; i < phases->GetSize(); i++)
    {
        BL_FLOAT prevPhase = prevUnwrapPhases.Get()[i];
        BLUtilsPhases::FindNextPhase(&prevPhase, (BL_FLOAT)0.0);
                
        BL_FLOAT phase = phases->Get()[i];
        BLUtilsPhases::FindNextPhase(&phase, prevPhase);
        phases->Get()[i] = phase;
    }
    
    mUnwrappedPhasesTime.push_back(*phases);
    while(mUnwrappedPhasesTime.size() > mHistorySize)
        mUnwrappedPhasesTime.pop_front();
}

// See: http://kth.diva-portal.org/smash/get/diva2:1381398/FULLTEXT01.pdf
// "Pitch-shifting algorithm design and applications in musicé - THEO ROYER
// (phase unwrapping in time very well explained)
void
PhasesUnwrapper::ComputeUwPhasesDiffTime(WDL_TypedBuf<BL_FLOAT> *diff,
                                         const WDL_TypedBuf<BL_FLOAT> &phases0,
                                         const WDL_TypedBuf<BL_FLOAT> &phases1,
                                         BL_FLOAT sampleRate, int bufferSize,
                                         int overlapping)
{
    diff->Resize(phases0.GetSize());

    BL_FLOAT hzPerBin = sampleRate/bufferSize;
    // Interval between 2 measurements
    BL_FLOAT h = (bufferSize/sampleRate)/overlapping;
        
    for (int i = 0; i < diff->GetSize(); i++)
    {
        BL_FLOAT p0 = phases0.Get()[i];
        BL_FLOAT p1 = phases1.Get()[i];

        // Frequency of current bin
        BL_FLOAT fk = i*hzPerBin;

        // Origin
        //BL_FLOAT omegaK = h*2.0*M_PI*fk;

        //BL_FLOAT omegaK = (2.0*M_PI*overlapping*i)/bufferSize;
        //BL_FLOAT omegaK = (2.0*M_PI*(bufferSize/overlapping)*i)/bufferSize;
        //BL_FLOAT omegaK = 2.0*M_PI*bufferSize/overlapping*i*bufferSize/2;

        // From: https://github.com/eevah/matlab/blob/master/VX_pitch_pv.m
        //
        // and maybe also:
        // https://dadorran.wordpress.com/2014/06/02/audio-time-scale-modification-phase-vocoder-implementation-in-matlab
        //
        //BL_FLOAT omegaK = 2.0*M_PI*(bufferSize/overlapping)*
        //    ((BL_FLOAT)i)/diff->GetSize();
        BL_FLOAT omegaK = 2.0*M_PI*(bufferSize/overlapping)*
            ((BL_FLOAT)i)/bufferSize;
        
        BL_FLOAT dp = omegaK + BLUtilsPhases::princarg(p1 - p0 - omegaK);
        
        diff->Get()[i] = dp;
    }
}

void
PhasesUnwrapper::NormalizePhasesTime(WDL_TypedBuf<BL_FLOAT> *phases)
{
    if (mUnwrappedPhasesTime.empty())
    {
        BLUtils::Normalize(phases);
        
        return;
    }
    
    const WDL_TypedBuf<BL_FLOAT> &oldPhases = mUnwrappedPhasesTime[0];
    const WDL_TypedBuf<BL_FLOAT> &newPhases =
                            mUnwrappedPhasesTime[mUnwrappedPhasesTime.size() - 1];
    
#if !NORMALIZE_TIME_PER_BIN
    BL_FLOAT min = BLUtils::ComputeMin(oldPhases);
    BL_FLOAT max = BLUtils::ComputeMax(newPhases);
    
    BLUtils::Normalize(phases, min, max);
#else

#if TIME_TAKE_MIDDLE_VALUE
    // GOOD: but we will have a shift in time
    int middle = mUnwrappedPhasesTime.size()/2;
    
    if (middle < 0)
        middle = 0;
    const WDL_TypedBuf<BL_FLOAT> &middlePhases = mUnwrappedPhasesTime[middle];
#endif
    for (int i = 0; i < phases->GetSize(); i++)
    {
        BL_FLOAT minimum = oldPhases.Get()[i];
        BL_FLOAT maximum = newPhases.Get()[i];
        
#if !TIME_TAKE_MIDDLE_VALUE
        // If we don't take the middle, phase wil always be the maximum
        //  (always 1 after normalization)
        // and we won't get display
        BL_FLOAT phase = phases->Get()[i];
#else
        BL_FLOAT phase = middlePhases.Get()[i];
#endif
        
        phase = BLUtils::Normalize(phase, minimum, maximum);

        phases->Get()[i] = phase;
    }
#endif
}

void
PhasesUnwrapper::UnwrapPhasesTime(const WDL_TypedBuf<BL_FLOAT> &phases0,
                                  WDL_TypedBuf<BL_FLOAT> *phases1)
{
    for (int i = 0; i < phases1->GetSize(); i++)
    {
        BL_FLOAT p0 = phases0.Get()[i];
        //BLUtilsPhases::FindNextPhase(&p0, (BL_FLOAT)0.0);
                
        BL_FLOAT p1 = phases1->Get()[i];
        BLUtilsPhases::FindNextPhase(&p1, p0);

        phases1->Get()[i] = p1;
    }
}

void
PhasesUnwrapper::ComputePhasesGradientTime(WDL_TypedBuf<BL_FLOAT> *phases)
{
    if (mUnwrappedPhasesTime.size() < 2)
    {
        BLUtils::FillAllZero(phases);
        
        return;
    }
    
    const WDL_TypedBuf<BL_FLOAT> &currentPhases = *phases;
    const WDL_TypedBuf<BL_FLOAT> &prevPhases =
                    mUnwrappedPhasesTime[mUnwrappedPhasesTime.size() - 2];
    
    phases->Get()[0] = 0.0;
    for (int i = 1; i < currentPhases.GetSize(); i++)
    {
        BL_FLOAT prevPhase = prevPhases.Get()[i];
        BL_FLOAT currentPhase = currentPhases.Get()[i];
        
        BL_FLOAT diff = std::fabs(currentPhase - prevPhase);
        phases->Get()[i] = diff;
    }
}

void
PhasesUnwrapper::NormalizePhasesGradientTime(WDL_TypedBuf<BL_FLOAT> *phases)
{
     BLUtils::Normalize(phases);
}
