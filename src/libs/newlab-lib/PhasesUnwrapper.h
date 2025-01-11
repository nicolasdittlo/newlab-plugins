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

#ifndef PHASES_UNWRAPPER_H
#define PHASES_UNWRAPPER_H

#include <deque>
using namespace std;

#include "IPlug_include_in_plug_hdr.h"

class PhasesUnwrapper
{
public:
    PhasesUnwrapper(long historySize);
    
    virtual ~PhasesUnwrapper();

    void Reset();
    
    void SetHistorySize(long historySize);
    
    // Along freqs
    static void UnwrapPhasesFreq(WDL_TypedBuf<BL_FLOAT> *phases);
    void NormalizePhasesFreq(WDL_TypedBuf<BL_FLOAT> *phases);
    
    // Must call UnwrapPhasesFreq before
    void ComputePhasesGradientFreqs(WDL_TypedBuf<BL_FLOAT> *phases);
    void NormalizePhasesGradientFreqs(WDL_TypedBuf<BL_FLOAT> *phases);
    
    // Along time
    void UnwrapPhasesTime(WDL_TypedBuf<BL_FLOAT> *phases);
    void NormalizePhasesTime(WDL_TypedBuf<BL_FLOAT> *phases);

    static void UnwrapPhasesTime(const WDL_TypedBuf<BL_FLOAT> &phases0,
                                 WDL_TypedBuf<BL_FLOAT> *phases1);

    // See: http://kth.diva-portal.org/smash/get/diva2:1381398/FULLTEXT01.pdf
    static void ComputeUwPhasesDiffTime(WDL_TypedBuf<BL_FLOAT> *diff,
                                        const WDL_TypedBuf<BL_FLOAT> &phases0,
                                        const WDL_TypedBuf<BL_FLOAT> &phases1,
                                        BL_FLOAT sampleRate, int bufferSize,
                                        int overlapping);
    
    // Must call UnwrapPhasesTime() before
    void ComputePhasesGradientTime(WDL_TypedBuf<BL_FLOAT> *phases);
    void NormalizePhasesGradientTime(WDL_TypedBuf<BL_FLOAT> *phases);
    
protected:
    long mHistorySize;
    
    deque<WDL_TypedBuf<BL_FLOAT> > mUnwrappedPhasesTime;
    deque<WDL_TypedBuf<BL_FLOAT> > mUnwrappedPhasesFreqs;
    
    // For FREQ_GLOBAL_MIN_MAX
    BL_FLOAT mGlobalMinDiff;
    BL_FLOAT mGlobalMaxDiff;
};

#endif
