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

#ifndef SNT_ALGO_H
#define SNT_ALGO_H

#include <vector>
#include <deque>
using namespace std;

#include "bl_queue.h"

class STNAlgo
{
 public:
    STNAlgo();
    virtual ~STNAlgo();

    void reset();
    
    // TODO: optimize with bl_queue

    // For the time axis (nMedianH),
    // process only one horizontal median filter window centered on the middle column of X,
    // and return this processing as one result column.
    void transientness(const bl_queue<vector<float> > &X,
                       int nMedianH, int nMedianV,
                       vector<float> *result);
    
    static void computeNMedian(int fftSize, int overlap, float sampleRate, int *nMedianH, int *nMedianV);

    static void decSTN(const vector<float> &Rt, float G2, float G1,
                       vector<float> *S, vector<float> *T, vector<float> *N);
        
protected:
    // Freq axis
    static void medfilt1_v(const bl_queue<vector<float> > &X, int nMedianV, int col, vector<float> *result);

    // Time axis
    void medfilt1_h(const bl_queue<vector<float> > &X, int nMedianH, int col, vector<float> *result);

    vector<vector<float> > _hWins;
    vector<bl_queue<float> > _hValuesHistories;
};

#endif
