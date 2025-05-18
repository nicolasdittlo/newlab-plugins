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
#include "STNAlgo.h"

#define MEDFILT_V_OPTIM 1
#define MEDFILT_H_OPTIM 1

template<typename T>
typename std::vector<T>::iterator 
insert_sorted(std::vector<T> &vec, T const &item)
{
    return vec.insert(std::upper_bound(vec.begin(), vec.end(), item), item);
}

template<typename T>
void
remove_sorted(std::vector<T> &vec, T const &item)
{
    auto lb = std::lower_bound(vec.begin(), vec.end(), item);
    vec.erase(lb);
}

STNAlgo::STNAlgo() {}

STNAlgo::~STNAlgo() {}

void
STNAlgo::reset()
{
    _hWins.clear();
    _hValuesHistories.clear();
}

void
STNAlgo::transientness(const bl_queue<vector<float> > &X,
                       int nMedianH, int nMedianV,
                       vector<float> *result)
{
    int col = nMedianH/2/* + 1*/;
    
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
STNAlgo::computeNMedian(int fftSize, int overlap, float sampleRate, int *nMedianH, int *nMedianV)
{
    float filter_length_t = 200e-3; // in ms
    float filter_length_f = 500.0; // in Hz
    
    int nHop = fftSize/overlap;
    *nMedianH = round(filter_length_t * sampleRate / nHop);
    *nMedianV = round(filter_length_f * fftSize / sampleRate);
}

static float
fast_sin (float x)
{
    auto x2 = x * x;
    auto numerator = -x * (-11511339840 + x2 * (1640635920 + x2 * (-52785432 + x2 * 479249)));
    auto denominator = 11511339840 + x2 * (277920720 + x2 * (3177720 + x2 * 18361));
    return numerator / denominator;
}

void
STNAlgo::decSTN(const vector<float> &Rt, float G2, float G1,
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

    float PiG1G2Inv = M_PI/(2.0*(G1 - G2));
                      
    // S = sin(pi*(Rs-G2)/(2*(G1-G2))).^2;
    // S(Rs>=G1) = 1; S(Rs<G2) = 0;
    S->resize(Rs.size());
    for (int i = 0; i < S->size(); i++)
    {
        (*S)[i] = fast_sin((Rs[i] - G2)*PiG1G2Inv);
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
        (*T)[i] = fast_sin((Rt[i] - G2)*PiG1G2Inv);
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

#if !MEDFILT_V_OPTIM
// Freq axis
void
STNAlgo::medfilt1_v(const bl_queue<vector<float> > &X, int nMedianV, int col, vector<float> *result)
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
            if ((j >= 0) && (j < freqs.size()))
                win[j - i + nMedianV/2] = freqs[j];
            else
                win[j - i + nMedianV/2] = 0.0;                              
        }
        
        auto m = win.begin() + win.size() / 2;
        std::nth_element(win.begin(), m, win.end());

        (*result)[i] = win[win.size()/2];
    }
}

#else //MEDFILT_V_OPTIM

// Freq axis
void
STNAlgo::medfilt1_v(const bl_queue<vector<float> > &X, int nMedianV, int col, vector<float> *result)
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
    bl_queue<float> valuesHistory;

    int halfMedianV = nMedianV/2;
    
    for (int i = 0; i < freqs.size(); i++)
    {
        if (i == 0)
            // First time, fill the whole window
        {
            for (int j = i - halfMedianV; j < i + halfMedianV; j++)
            {
                if ((j >= 0) && (j < freqs.size()))
                {
                    insert_sorted(win, freqs[j]);
                    valuesHistory.push_back(freqs[j]);
                }
                else
                {
                    insert_sorted(win, 0.0f);
                    valuesHistory.push_back(0.0);
                }
            }

            valuesHistory.freeze();
        }
        else
        {
            float newValue = 0.0;
            if (i + halfMedianV - 1 < freqs.size())
                newValue = freqs[i + halfMedianV - 1];

            insert_sorted(win, newValue);
            float oldestValue = valuesHistory[0];
                        
            valuesHistory.push_pop(newValue);

            remove_sorted(win, oldestValue);
        }

        (*result)[i] = win[win.size()/2];
    }
}
#endif

#if !MEDFILT_H_OPTIM
// Time axis
void
STNAlgo::medfilt1_h(const bl_queue<vector<float> > &X, int nMedianH, int col, vector<float> *result)
{
    if (X.empty())
        return;

    result->resize(X[0].size());

    vector<float> win;
    win.resize(nMedianH);
        
    for (int i = 0; i < result->size(); i++)
    {
        for (int j = col - nMedianH/2; j < col + nMedianH/2; j++)
        {
            if ((j >= 0) && (j < X.size()))
                win[j - col + nMedianH/2] = X[j][i];
            else
                win[j - col + nMedianH/2] = 0.0;
        }

        auto m = win.begin() + win.size() / 2;
        std::nth_element(win.begin(), m, win.end());
        
        (*result)[i] = win[win.size()/2];
    }
}

#else // MEDFILT_H_OPTIM

// Time axis
void
STNAlgo::medfilt1_h(const bl_queue<vector<float> > &X, int nMedianH, int col, vector<float> *result)
{
    if (X.empty())
        return;

    result->resize(X[0].size());
    Utils::fillZero(result);
    
    if (_hWins.empty())
        // init, fill the windows
    {
        _hWins.resize(result->size());
        _hValuesHistories.resize(result->size());
        
        for (int i = 0; i < result->size(); i++)
        {    
            for (int j = col - nMedianH/2; j < col + nMedianH/2; j++)
            {
                if ((j >= 0) && (j < X.size()))
                {
                    float newValue = X[j][i];
                    insert_sorted(_hWins[i], newValue);
                    _hValuesHistories[i].push_back(newValue);
                }
                else
                {
                    insert_sorted(_hWins[i], 0.0f);
                    _hValuesHistories[i].push_back(0.0f);
                }
            }

            (*result)[i] = _hWins[i][_hWins[i].size()/2];

            _hValuesHistories[i].freeze();
        }
    }
    else
    {
        for (int i = 0; i < result->size(); i++)
        {
            float newValue = 0.0;
            // The newest value is X[0]
            //if (!X.empty())
            //    newValue = X[0][i];

            // The newest value is on the right
            if (!X.empty())
                newValue = X[X.size() - 1][i];
                
            insert_sorted(_hWins[i], newValue);

            float oldestValue = _hValuesHistories[i][0];
            
            _hValuesHistories[i].push_pop(newValue);

            remove_sorted(_hWins[i], oldestValue);
            
            (*result)[i] = _hWins[i][_hWins[i].size()/2];
        }
    }
}
#endif
