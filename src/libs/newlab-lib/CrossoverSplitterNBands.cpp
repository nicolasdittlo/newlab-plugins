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

#include "FilterRBJ2X.h"
#include "FilterTransparentRBJ2X.h"

#include "CrossoverSplitterNBands.h"

CrossoverSplitterNBands::CrossoverSplitterNBands(int numBands,
                                                 float cutoffFreqs[],
                                                 float sampleRate)
{
    _numBands = numBands;
    
    _sampleRate = sampleRate;
    
    if (numBands > 1)
    {
        _filterChains.resize(_numBands);
        
        _cutoffFreqs.resize(_numBands - 1);
        for (int i = 0; i < _numBands - 1; i++)
            _cutoffFreqs[i] = cutoffFreqs[i];
        
        createFilters(_sampleRate);
    }
    
    _tmpResultCross = new float[_numBands];
    _tmpResultCross2 = new float[_numBands];
}

CrossoverSplitterNBands::CrossoverSplitterNBands(const CrossoverSplitterNBands &other)
{
    _numBands = other._numBands;
    _sampleRate = other._sampleRate;
    
    if (_numBands > 1)
    {
        _filterChains.resize(_numBands);
        
        _cutoffFreqs.resize(_numBands - 1);
        for (int i = 0; i < _numBands - 1; i++)
            _cutoffFreqs[i] = other._cutoffFreqs[i];
        
        createFilters(_sampleRate);
    }
    
    _tmpResultCross = new float[_numBands];
    _tmpResultCross2 = new float[_numBands];
}

CrossoverSplitterNBands::~CrossoverSplitterNBands()
{
    for (int i = 0; i < _filterChains.size(); i++)
    {
        const vector<FilterRBJ *> &chain = _filterChains[i];
        for (int j = 0; j < chain.size(); j++)
        {
            FilterRBJ *filter = chain[j];
            
            delete filter;
        }
    }
    
    delete []_tmpResultCross;
    delete []_tmpResultCross2;
}

void
CrossoverSplitterNBands::reset(float sampleRate)
{
    _sampleRate = sampleRate;
    
    setFiltersValues();
}

void
CrossoverSplitterNBands::setCutoffFreqs(float freqs[])
{
    for (int j = 0; j < _numBands - 1; j++)
        _cutoffFreqs[j] = freqs[j];
    
    setFiltersValues();
}

int
CrossoverSplitterNBands::getNumBands()
{
    return _numBands;
}

float
CrossoverSplitterNBands::getCutoffFreq(int freqNum)
{
    if (freqNum >= _numBands - 1)
        return -1.0;
    
    float freq = _cutoffFreqs[freqNum];
    
    return freq;
}

void
CrossoverSplitterNBands::setCutoffFreq(int freqNum, float freq)
{
    if (freqNum >= _numBands - 1)
        return;
    
    _cutoffFreqs[freqNum] = freq;
    
    // Not optimized, set the values for all the filters
    // (and not only the ones with cutoff)
    setFiltersValues();
}

void
CrossoverSplitterNBands::split(float sample, float result[])
{    
    for (int i = 0; i < _filterChains.size(); i++)
    {
        const vector<FilterRBJ *> &chain = _filterChains[i];
        
        for (int j = 0; j < chain.size(); j++)
        {
            if ((i == 0) && (j == 0))
                // First chain, first filter
            {
                _tmpResultCross[i] = sample;
                
                FilterRBJ *filter = _filterChains[i][j];
                result[i] = filter->process(sample);
            }
            else if (j == 0)
                // Other chains, first filter
            {
                FilterRBJ *filter = _filterChains[i][j];
                result[i] = filter->process(_tmpResultCross[i - 1]);
                _tmpResultCross[i] = result[i];
            }
            else
                // Next filters
            {
                FilterRBJ *filter = _filterChains[i][j];
                result[i] = filter->process(result[i]);
            }
        }
    }
}

void
CrossoverSplitterNBands::split(const vector<float> &samples,
                               vector<float> result[])
{
    for (int i = 0; i < _numBands; i++)
    {
        result[i].resize(samples.size());
    }
    
    //WDL_TypedBuf<float> resultCross[mNumBands];
    vector<vector<float> > &resultCross = _tmpBuf0;
    resultCross.resize(_numBands);

    for (int i = 0; i < _filterChains.size(); i++)
    {
        const vector<FilterRBJ *> &chain = _filterChains[i];
            
        for (int j = 0; j < chain.size(); j++)
        {
            if ((i == 0) && (j == 0))
                // First chain, first filter
            {
                resultCross[i] = samples;
                
                FilterRBJ *filter = _filterChains[i][j];
                
                result[i] = samples;
                filter->process(&result[i]);
            }
            else if (j == 0)
                // Other chains, first filter
            {
                FilterRBJ *filter = _filterChains[i][j];
                
                result[i] = resultCross[i - 1];
                
                filter->process(&result[i]);
                
                resultCross[i] = result[i];
            }
            else
                // Next filters
            {
                FilterRBJ *filter = _filterChains[i][j];
                filter->process(&result[i]);
            }
        }
    }
}

void
CrossoverSplitterNBands::createFilters(float sampleRate)
{
    for (int i = 0; i < _numBands; i++)
    {
        if (i == 0)
        {
            FilterRBJ2X *filter =
                new FilterRBJ2X(FILTER_TYPE_LOWPASS,
                                    _sampleRate,
                                   _cutoffFreqs[i]);
            _filterChains[i].push_back(filter);
        }
        else if (i == _numBands - 1)
        {
            FilterRBJ2X *filter =
                new FilterRBJ2X(FILTER_TYPE_HIPASS,
                                _sampleRate,
                                _cutoffFreqs[i - 1]);
            _filterChains[i].push_back(filter);
        }
        else
        {
            FilterRBJ2X *filter =
                new FilterRBJ2X(FILTER_TYPE_HIPASS,
                                _sampleRate,
                                _cutoffFreqs[i - 1]);
            _filterChains[i].push_back(filter);
            
            FilterRBJ2X *filter2 =
                new FilterRBJ2X(FILTER_TYPE_LOWPASS,
                                _sampleRate,
                                _cutoffFreqs[i]);
            _filterChains[i].push_back(filter2);
        }
    }
    
    // With that, we got flat curve when injecting IRs
    for (int i = 0; i < _numBands; i++)
    {
        int numTransparentFilters = _numBands - i - 2;
        if (numTransparentFilters < 0)
            numTransparentFilters = 0;

        for (int j = 0; j < numTransparentFilters; j++)
        {
            FilterTransparentRBJ2X *filter =
                new FilterTransparentRBJ2X(_sampleRate,
                                           _cutoffFreqs[i + j + 1]);
            
            _filterChains[i].push_back(filter);
        }
    }
}

void
CrossoverSplitterNBands::setFiltersValues()
{
    for (int i = 0; i < _numBands; i++)
    {
        if (i == 0)
        {
            FilterRBJ *filter = _filterChains[i][0];
            filter->setCutoffFreq(_cutoffFreqs[i]);
            filter->setSampleRate(_sampleRate);
        }
        else if (i == _numBands - 1)
        {
            FilterRBJ *filter = _filterChains[i][0];
            filter->setCutoffFreq(_cutoffFreqs[i - 1]);
            filter->setSampleRate(_sampleRate);
        }
        else
        {
            FilterRBJ *filter = _filterChains[i][0];
            filter->setCutoffFreq(_cutoffFreqs[i - 1]);
            filter->setSampleRate(_sampleRate);
      
            FilterRBJ *filter2 = _filterChains[i][1];
            filter2->setCutoffFreq(_cutoffFreqs[i]);
            filter2->setSampleRate(_sampleRate);
        }
    }
    
    for (int i = 0; i < _numBands; i++)
    {
        int numTransparentFilters = _numBands - i - 2;
        if (numTransparentFilters < 0)
            numTransparentFilters = 0;
        
        for (int j = 0; j < numTransparentFilters; j++)
        {
            int index = _filterChains[i].size() - 1 - j;
            FilterRBJ *filter = _filterChains[i][index];
            
            int freqIndex = _cutoffFreqs.size() - 1 - j;
            filter->setCutoffFreq(_cutoffFreqs[freqIndex]);
            filter->setSampleRate(_sampleRate);
        }
    }
}
