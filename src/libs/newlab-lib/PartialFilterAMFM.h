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

#ifndef PARTIAL_FILTER_AMFM_H
#define PARTIAL_FILTER_AMFM_H

#include <vector>
#include <deque>
using namespace std;

#include "PartialFilter.h"

// Method using alhpa0 and delta0 from AM FM paper
// https://www.researchgate.net/publication/235219224_Improved_partial_tracking_technique_for_sinusoidal_modeling_of_speech_and_audio
// Also use zombied as suggested
class PartialFilterAMFM : public PartialFilter
{
 public:
    PartialFilterAMFM(int bufferSize, float sampleRate);
    virtual ~PartialFilterAMFM();

    void reset(int bufferSize, float smapleRate);
        
    void filterPartials(vector<Partial> *partials);

    void setNeriDelta(float delta);
    
 protected:
    // Method based on alpha0 and beta0
    void associatePartialsAMFM(const vector<Partial> &prevPartials,
                               vector<Partial> *currentPartials,
                               vector<Partial> *remainingCurrentPartials);
    long findNearestFreqId(const vector<Partial> &partials,
                           float freq, int index);
        
    // Compute score using Neri
    void associatePartialsNeri(const vector<Partial> &prevPartials,
                               vector<Partial> *currentPartials,
                               vector<Partial> *remainingCurrentPartials);
    
    void computeZombieDeadPartials(const vector<Partial> &prevPartials,
                                   const vector<Partial> &currentPartials,
                                   vector<Partial> *zombieDeadPartials);

    void fixPartialsCrossing(const vector<Partial> &partials0,
                             const vector<Partial> &partials1,
                             vector<Partial> *partials2);
        
    int findPartialById(const vector<Partial> &partials, int idx);
    // Optimized
    int findPartialByIdSorted(const vector<Partial> &partials,
                              const Partial &refPartial);
    
    // For AMFM
    float ComputeLA(const Partial &prevPartial, const Partial &currentPartial);
    float ComputeLF(const Partial &prevPartial, const Partial &currentPartial);

    void computeCostNeri(const Partial &prevPartial,
                         const Partial &currentPartial,
                         float delta, float zetaF, float zetaA,
                         float *A, float *B);

    bool checkDiscardBigJump(const Partial &prevPartial,
                             const Partial &currentPartial);
    bool checkDiscardOppositeDirection(const Partial &prevPartial,
                                       const Partial &currentPartial);
    
    deque<vector<Partial> > _partials;

    int _bufferSize;
    float _sampleRate;

    float _neriDelta;
    
 private:
    vector<Partial> _tmpPartials0;
    vector<Partial> _tmpPartials1;
    vector<Partial> _tmpPartials2;
    vector<Partial> _tmpPartials3;
    vector<Partial> _tmpPartials4;
    vector<Partial> _tmpPartials5;
    vector<Partial> _tmpPartials6;
    vector<Partial> _tmpPartials7;
};

#endif
