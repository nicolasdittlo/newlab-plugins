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

// No optimization
#define MEDFILT_V_NO_OPTIM 0
#define MEDFILT_H_NO_OPTIM 0

// Custom optimization
#define MEDFILT_V_OPTIM 1 //0
#define MEDFILT_H_OPTIM 1 //0

#define MEDFILT_V_OPTIM2 0 //1
#define MEDFILT_H_OPTIM2 0 //1


#if (defined MEDFILT_V_OPTIM) || (defined MEDFILT_H_OPTIM)
template<typename T>
void
insert_sorted(std::vector<T> &vec, T const &item)
{
    vec.insert(std::upper_bound(vec.begin(), vec.end(), item), item);
}

template<typename T>
void
remove_sorted(std::vector<T> &vec, T const &item)
{
    vec.erase(std::lower_bound(vec.begin(), vec.end(), item));
}
#endif


#if (defined MEDFILT_V_OPTIM2) || (defined MEDFILT_H_OPTIM2)

#define HISTO_SIZE 8192 //131072 //8192

#if 1 //0 // orig
// This method cuts sines high frequencies, but preserve transients high frequencies
//
// (think to modify the instantiation)

class HistogramMedianFilter
{
public:
    HistogramMedianFilter(int numBins = 129, int windowSize = 51,
                          float minValue = -1.0f, float maxValue = 1.0f)
    : _binCount(numBins), _windowSize(windowSize),
      _minValue(minValue), _maxValue(maxValue),
      _histogram(numBins, 0), _buffer(windowSize, 0.0f), _bufferIndex(0), _count(0)
    {
        _binWidth = (_maxValue - _minValue) / _binCount;
    }

    float process(float sample)
    {   
        // Remove oldest sample from histogram
        if (_count >= _windowSize)
        {
            int oldBin = sampleToBin(_buffer[_bufferIndex]);
            _histogram[oldBin]--;
        }
        else
        {
            _count++;
        }

        // Add new sample to buffer and histogram
        _buffer[_bufferIndex] = sample;
        int newBin = sampleToBin(sample);
        _histogram[newBin]++;
        _bufferIndex = (_bufferIndex + 1) % _windowSize;

        float result = getMedianFromHistogram();
        
        return result;
    }

    void reset(float value = 0.0f)
    {
        std::fill(_histogram.begin(), _histogram.end(), 0);
        std::fill(_buffer.begin(), _buffer.end(), value);
        _count = 0;
        _bufferIndex = 0;
    }

private:
    int _binCount;
    int _windowSize;
    int _bufferIndex;
    int _count;
    float _minValue;
    float _maxValue;
    float _binWidth;

    std::vector<int> _histogram;
    std::vector<float> _buffer;

    int sampleToBin(float sample) const
    {
        int bin = static_cast<int>((sample - _minValue) / _binWidth);
        return std::clamp(bin, 0, _binCount - 1);
    }

    float getMedianFromHistogram() const
    {
        int target = _count / 2;
        int cumulative = 0;
        for (int i = 0; i < _binCount; ++i)
        {
            cumulative += _histogram[i];
            if (cumulative > target)
            {
                // Return center of bin
                return _minValue + (i + 0.5f) * _binWidth;
            }
        }
        return _maxValue; // Fallback (should never hit)
    }
};
#endif

// This method gives more high frequencies for sines (which is good), but cuts the transients high frequencies.
//
// (think to modify the instantiation)
#if 0 //1 //0 //
class HistogramMedianFilter
{
public:
    HistogramMedianFilter(size_t windowSize, size_t numBins = 8192)
        : windowSize(windowSize),
          numBins(numBins),
          histogram(numBins, 0),
          buffer(windowSize, 0),
          insertIndex(0),
          count(0)
    {}

    // TODO
    void reset() {}
    
    float process(float input)
    {
        // Quantize input to bin index
        uint16_t newBin = quantize(input);

        // Remove oldest value
        if (count == windowSize) {
            uint16_t oldBin = buffer[insertIndex];
            histogram[oldBin]--;
        } else {
            count++;
        }

        // Insert new value
        buffer[insertIndex] = newBin;
        histogram[newBin]++;
        insertIndex = (insertIndex + 1) % windowSize;

        // Return dequantized median
        return dequantize(findMedianBin());
    }

private:
    size_t windowSize;
    size_t numBins;
    std::vector<uint16_t> buffer;
    std::vector<uint32_t> histogram;
    size_t insertIndex;
    size_t count;

    uint16_t quantize(float value) const
    {
        value = std::clamp(value, 0.0f, 1.0f);
        return static_cast<uint16_t>(value * (numBins - 1) + 0.5f);
    }

    float dequantize(uint16_t bin) const
    {
        return static_cast<float>(bin) / static_cast<float>(numBins - 1);
    }

    uint16_t findMedianBin() const
    {
        size_t target = count / 2;
        size_t cumulative = 0;
        for (size_t i = 0; i < numBins; ++i) {
            cumulative += histogram[i];
            if (cumulative > target)
                return static_cast<uint16_t>(i);
        }
        return static_cast<uint16_t>(numBins - 1); // Fallback
    }
};
#endif

#endif


STNAlgo::STNAlgo() {}

STNAlgo::~STNAlgo()
{
    for (int i = 0; i < _hFilters.size(); i++)
        delete _hFilters[i];
}

void
STNAlgo::reset()
{
    _hWins.clear();
    _hValuesHistories.clear();

    for (int i = 0; i < _hFilters.size(); i++)
        _hFilters[i]->reset();
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
    // G1 = 0.9;    // Upper threshold    
    // G2 = 0.75;   // Lower threshold
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

#if MEDFILT_V_NO_OPTIM
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
#endif

#if MEDFILT_V_OPTIM
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

#if MEDFILT_V_OPTIM2
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

    HistogramMedianFilter filter(HISTO_SIZE, nMedianV, 0.0, 1.0);
    //HistogramMedianFilter filter(nMedianV);
        
    for (int i = 0; i < freqs.size(); i++)
        (*result)[i] = filter.process(freqs[i]);
}
#endif

#if MEDFILT_H_NO_OPTIM
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
#endif

#if MEDFILT_H_OPTIM
// Time axis
void
STNAlgo::medfilt1_h(const bl_queue<vector<float> > &X, int nMedianH, int col, vector<float> *result)
{
    if (X.empty())
        return;

    result->resize(X[0].size());
    Utils::fillZero(result);

    int halfMedianH = nMedianH/2;
    
    if (_hWins.empty())
        // init, fill the windows
    {
        _hWins.resize(result->size());
        _hValuesHistories.resize(result->size());
        
        for (int i = 0; i < result->size(); i++)
        {    
            for (int j = col - halfMedianH; j < col + halfMedianH; j++)
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

#if MEDFILT_H_OPTIM2
// Time axis
void
STNAlgo::medfilt1_h(const bl_queue<vector<float> > &X, int nMedianH, int col, vector<float> *result)
{
    if (X.empty())
        return;

    result->resize(X[0].size());

    // TODO: make adaptive to nMedianH
    if (_hFilters.empty())
    {
        for (int i = 0; i < result->size(); i++)
            _hFilters.push_back(new HistogramMedianFilter(HISTO_SIZE, nMedianH, 0.0, 1.0));
        //_hFilters.push_back(new HistogramMedianFilter(nMedianH));
    }

    for (int i = 0; i < result->size(); i++)
        (*result)[i] = _hFilters[i]->process(/*X[0][i]*/X[X.size()-1][i]);
}
#endif
