/* Copyright (C) 2025 Nicolas Dittlo <bluelab.plugins@gmail.com>
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

class PhasesUnwrapper
{
public:
    PhasesUnwrapper(long historySize);
    
    virtual ~PhasesUnwrapper();

    void reset();
    
    void setHistorySize(long historySize);
    
    // Along freqs
    static void unwrapPhasesFreq(vector<float> *phases);
    void normalizePhasesFreq(vector<float> *phases);
    
    // Must call UnwrapPhasesFreq before
    void computePhasesGradientFreqs(vector<float> *phases);
    void normalizePhasesGradientFreqs(vector<float> *phases);
    
    // Along time
    void unwrapPhasesTime(vector<float> *phases);
    void normalizePhasesTime(vector<float> *phases);

    static void unwrapPhasesTime(const vector<float> &phases0,
                                 vector<float> *phases1);

    // See: http://kth.diva-portal.org/smash/get/diva2:1381398/FULLTEXT01.pdf
    static void computeUwPhasesDiffTime(vector<float> *diff,
                                        const vector<float> &phases0,
                                        const vector<float> &phases1,
                                        float sampleRate, int bufferSize,
                                        int overlapping);
    
    // Must call UnwrapPhasesTime() before
    void computePhasesGradientTime(vector<float> *phases);
    void normalizePhasesGradientTime(vector<float> *phases);
    
protected:
    long _historySize;
    
    deque<vector<float> > _unwrappedPhasesTime;
    deque<vector<float> > _unwrappedPhasesFreqs;
    
    float _globalMinDiff;
    float _globalMaxDiff;
};

#endif
