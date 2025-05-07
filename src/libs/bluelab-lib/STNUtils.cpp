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

#include <math.h>

#include <algorithm>
using namespace std;

#include "Defines.h"
#include "Utils.h"
#include "STNUtils.h"

void
STNUtils::computeNMedian(int fftSize, int overlap, float sampleRate, int *nMedianH, int *nMedianV)
{
    float filter_length_t = 200e-3; // in ms
    float filter_length_f = 500.0; // in Hz
    
    int nHop = fftSize/overlap;
    *nMedianH = round(filter_length_t * sampleRate / nHop);
    *nMedianV = round(filter_length_f * fftSize / sampleRate);
}

void
STNUtils::transientness(const deque<vector<float> > &X,
                        int nMedianH, int nMedianV,
                        vector<float> *result)
{
    int col = nMedianH/2 + 1;
    
    vector<float> X_v_median;
    medfilt1_v(X, nMedianV, col, &X_v_median);

    vector<float> X_h_median;
    medfilt1_h(X, nMedianH, col, &X_h_median);

    // X_v_median and X_h_median should have the same size

    result->resize(X_v_median.size());
    for (int i = 0; i < X_v_median.size(); i++)
    {
        float Y = 0.0;
        
        if (fabs(X_v_median[i] + X_h_median[i]) > BL_EPS)
            Y = X_v_median[i] / (X_v_median[i] + X_h_median[i]);

        (*result)[i] = Y;
    }
}

void
STNUtils::decSTN(const vector<float> &Rt, float G2, float G1,
                 vector<float> *S, vector<float> *T, vector<float> *N)
{
    //if nargin < 2
    //G1 = 0.9;    % Upper threshold    
    //G2 = 0.75;   % Lower threshold
    //end

    // Rs = 1-Rt
    vector<float> Rs;
    Rs.resize(Rt.size());
    for (int i = 0; i < Rs.size(); i++)
        Rs[i] = 1.0 - Rt[i];  

    // S = sin(pi*(Rs-G2)/(2*(G1-G2))).^2;
    // S(Rs>=G1) = 1; S(Rs<G2) = 0;
    S->resize(Rs.size());
    for (int i = 0; i < S->size(); i++)
    {
        (*S)[i] = sin(M_PI*(Rs[i] - G2)/(2.0*(G1 - G2)));
        (*S)[i] = (*S)[i]*(*S)[i];

        if (Rs[i] >= G1)
            (*S)[i] = 1.0;
        if (Rs[i] < G2)
            (*S)[i] = 0.0;
    }

    // T = sin(pi*(Rt-G2)/(2*(G1-G2))).^2; 
    // T(Rt>=G1) = 1; T(Rt<G2) = 0;
    T->resize(Rt.size());
    for (int i = 0; i < T->size(); i++)
    {
        (*T)[i] = sin(M_PI*(Rt[i] - G2)/(2.0*(G1 - G2)));
        (*T)[i] = (*T)[i]*(*T)[i];

        if (Rt[i] >= G1)
            (*T)[i] = 1.0;
        if (Rt[i] < G2)
            (*T)[i] = 0.0;
    }

    // N = zeros(size(S)); N = 1-S-T;
    N->resize(S->size());
    for (int i = 0; i < N->size(); i++)
        (*N)[i] = 1.0 - (*S)[i] - (*T)[i];
}

// Freq axis
void
STNUtils::medfilt1_v(const deque<vector<float> > &X, int nMedianV, int col, vector<float> *result)
{
    if (col >= X.size())
    {
        if (!X.empty())
        {
            result->resize(X[0].size());
            Utils::fillZero(result);
        }

        return;
    }
    
    const vector<float> freqs = X[col];

    result->resize(freqs.size());
                       
    vector<float> win;
    win.resize(nMedianV);
    
    for (int i = 0; i < freqs.size(); i++)
    {
        for (int j = i - nMedianV/2; j < i + nMedianV/2; j++)
        {
            if ((j >= 0) || (j < freqs.size()))
                win[j - i + nMedianV/2] = freqs[j];
            else
                win[j - i + nMedianV/2] = 0.0;                              
        }

        sort(win.begin(), win.end());

        (*result)[i] = win[win.size()/2];
    }
}

// Time axis
void
STNUtils::medfilt1_h(const deque<vector<float> > &X, int nMedianH, int col, vector<float> *result)
{
    if (X.empty())
        return;

    result->resize(X[0].size());

    for (int i = 0; i < result->size(); i++)
    {
        vector<float> win;
        win.resize(nMedianH);
        
        for (int j = col - nMedianH/2; j < col + nMedianH/2; j++)
        {
            if ((j >= 0) && (j < X.size()))
                win[j - col + nMedianH/2] = X[j][i];
            else
                win[j - col + nMedianH/2] = 0.0;
        }

        sort(win.begin(), win.end());

        (*result)[i] = win[win.size()/2];
    }
}
