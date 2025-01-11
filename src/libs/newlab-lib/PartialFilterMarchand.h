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
#include <deque>
using namespace std;

#include "PartialFilter.h"

// Method of Sylvain Marchand (~2001)
class PartialFilterMarchand : public PartialFilter
{
 public:
    PartialFilterMarchand(int bufferSize, float sampleRate);
    virtual ~PartialFilterMarchand();

    void reset(int bufferSize, float sampleRate);
        
    void filterPartials(vector<Partial> *partials);

 protected:
    // Simple method, based on frequencies only
    void associatePartials(const vector<Partial> &prevPartials,
                           vector<Partial> *currentPartials,
                           vector<Partial> *remainingPartials);
    
    // See: https://www.dsprelated.com/freebooks/sasp/PARSHL_Program.html#app:parshlapp
    // "Peak Matching (Step 5)"
    // Use fight/winner/loser
    void associatePartialsPARSHL(const vector<Partial> &prevPartials,
                                 vector<Partial> *currentPartials,
                                 vector<Partial> *remainingPartials);

    float getDeltaFreqCoeff(int binNum);

    int findPartialById(const vector<Partial> &partials, int idx);
    
    //
    deque<vector<Partial> > _partials;

    int _bufferSize;
    
 private:
    vector<Partial> _tmpPartials0;
    vector<Partial> _tmpPartials1;
    vector<Partial> _tmpPartials2;
    vector<Partial> _tmpPartials3;
    vector<Partial> _tmpPartials4;
    vector<Partial> _tmpPartials5;
    vector<Partial> _tmpPartials6;
};

#endif
