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

#ifndef PARTIAL_FILTER_MARCHAND_H
#define PARTIAL_FILTER_MARCHAND_H

#include <vector>
using namespace std;

#include <PartialFilter.h>

// Method of Sylvain Marchand (~2001)
class PartialFilterMarchand : public PartialFilter
{
 public:
    PartialFilterMarchand(int bufferSize, BL_FLOAT sampleRate);
    virtual ~PartialFilterMarchand();

    void Reset(int bufferSize, BL_FLOAT sampleRate);
        
    void FilterPartials(vector<Partial> *partials);

 protected:
    // Simple method, based on frequencies only
    void AssociatePartials(const vector<Partial> &prevPartials,
                           vector<Partial> *currentPartials,
                           vector<Partial> *remainingPartials);
    
    // See: https://www.dsprelated.com/freebooks/sasp/PARSHL_Program.html#app:parshlapp
    // "Peak Matching (Step 5)"
    // Use fight/winner/loser
    void AssociatePartialsPARSHL(const vector<Partial> &prevPartials,
                                 vector<Partial> *currentPartials,
                                 vector<Partial> *remainingPartials);

    BL_FLOAT GetDeltaFreqCoeff(int binNum);

    int FindPartialById(const vector<Partial> &partials, int idx);
    
    //
    deque<vector<Partial> > mPartials;

    int mBufferSize;
    
 private:
    vector<Partial> mTmpPartials0;
    vector<Partial> mTmpPartials1;
    vector<Partial> mTmpPartials2;
    vector<Partial> mTmpPartials3;
    vector<Partial> mTmpPartials4;
    vector<Partial> mTmpPartials5;
    vector<Partial> mTmpPartials6;
};

#endif
