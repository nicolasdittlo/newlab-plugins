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

#ifndef CROSSOVER_SPLITTER_NBANDS_H
#define CROSSOVER_SPLITTER_NBANDS_H

#include <vector>
using namespace std;

class FilterRBJ;
class CrossoverSplitterNBands
{
public:
    CrossoverSplitterNBands(int numBands, float cutoffFreqs[], float sampleRate);
    
    CrossoverSplitterNBands(const CrossoverSplitterNBands &other);
    
    virtual ~CrossoverSplitterNBands();
    
    void reset(float sampleRate);
    
    void setCutoffFreqs(float freqs[]);
    
    int getNumBands();
    
    float getCutoffFreq(int freqNum);
    void setCutoffFreq(int freqNum, float freq);
    
    void split(float sample, float result[]);
    
    void split(const vector<float> &samples,
               vector<float> result[]);
    
protected:
    void createFilters(float sampleRate);
    
    void setFiltersValues();
    
    float _sampleRate;
    
    int _numBands;
    vector<float> _cutoffFreqs;
    
    vector<vector<FilterRBJ *> > _filterChains;
    
    float *_tmpResultCross;
    float *_tmpResultCross2;

private:
    vector<vector<float> > _tmpBuf0;
};

#endif
