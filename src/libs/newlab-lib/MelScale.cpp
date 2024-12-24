#include <algorithm>
using namespace std;

#include "Utils.h"

#include "MelScale.h"

// Filter bank
MelScale::FilterBank::FilterBank(int dataSize, float sampleRate, int numFilters)
{
    _dataSize = dataSize;
    _sampleRate = sampleRate;
    _numFilters = numFilters;
    
    _filters.resize(_numFilters);
    for (int i = 0; i < _filters.size(); i++)
    {
        _filters[i]._data.resize(dataSize);
        Utils::FillAllZero(&_filters[i]._data);
        
        _filters[i]._bounds[0] = -1;
        _filters[i]._bounds[1] = -1;
    }
}

MelScale::FilterBank::FilterBank()
{
    _dataSize = 0;
    _sampleRate = 0.0;
    _numFilters = 0;
}

MelScale::FilterBank::~FilterBank() {}

//

// See: http://practicalcryptography.com/miscellaneous/machine-learning/guide-mel-frequency-cepstral-coefficients-mfccs/

MelScale::MelScale() {}

MelScale::~MelScale() {}

float
MelScale::hzToMel(float freq)
{
    float mel = 2595.0*log10((float)(1.0 + freq/700.0));
    
    return mel;
}

float
MelScale::melToHz(float mel)
{
    float hz = 700.0*(pow((float)10.0, (float)(mel/2595.0)) - 1.0);
    
    return hz;
}

void
MelScale::hzToMel(vector<float> *resultMagns,
                  const vector<float> &magns,
                  float sampleRate)
{
    // For dB
    resultMagns->resize(magns.size());
    Utils::fillZero(resultMagns);
    
    float maxFreq = sampleRate*0.5;
    if (maxFreq < BL_EPS)
        return;
    
    float maxMel = hzToMel(maxFreq);
    
    float melCoeff = maxMel/resultMagns->size();
    float idCoeff = (1.0/maxFreq)*resultMagns->size();
    
    int resultMagnsSize = resultMagns->size();
    float *resultMagnsData = resultMagns->data();
    
    int magnsSize = magns.size();
    float *magnsData = magns.data();
    for (int i = 0; i < resultMagnsSize; i++)
    {
        float mel = i*melCoeff;
        float freq = melToHz(mel);
        
        float id0 = freq*idCoeff;
        
        int id0i = (int)id0;
        
        float t = id0 - id0i;
        
        if (id0i >= magnsSize)
            continue;
        
        // NOTE: this optim doesn't compute exactly the same thing than the original version
        int id1 = id0i + 1;
        if (id1 >= magnsSize)
            continue;
        
        float magn0 = magnsData[id0i];
        float magn1 = magnsData[id1];
        
        float magn = (1.0 - t)*magn0 + t*magn1;
        
        resultMagnsData[i] = magn;
    }
}

void
MelScale::melToHz(vector<float> *resultMagns,
                  const vector<float> &magns,
                  float sampleRate)
{
    resultMagns->resize(magns.GetSize());
    Utils::fillZero(resultMagns);
    
    float hzPerBin = sampleRate*0.5/magns.size();
    
    float maxFreq = sampleRate*0.5;
    float maxMel = hzToMel(maxFreq);
    
    int resultMagnsSize = resultMagns->size();
    float *resultMagnsData = resultMagns->data();
    int magnsSize = magns.size();
    float *magnsData = magns.data();
    
    for (int i = 0; i < resultMagnsSize; i++)
    {
        float freq = hzPerBin*i;
        float mel = hzToMel(freq);
        
        float id0 = (mel/maxMel) * resultMagnsSize;
        
        if ((int)id0 >= magnsSize)
            continue;
        
        float t = id0 - (int)(id0);
        
        int id1 = id0 + 1;
        if (id1 >= magnsSize)
            continue;
        
        float magn0 = magnsData[(int)id0];
        float magn1 = magnsData[id1];
        
        float magn = (1.0 - t)*magn0 + t*magn1;
        
        resultMagnsData[i] = magn;
    }
}

void
MelScale::hzToMelFilter(vector<float> *result,
                        const vector<float> &magns,
                        float sampleRate, int numFilters)
{
    if ((magns.size() != _hzToMelFilterBank._dataSize) ||
        (sampleRate != _hzToMelFilterBank._sampleRate) ||
        (numFilters != _hzToMelFilterBank._numFilters))
    {
        createFilterBankHzToMel(&_hzToMelFilterBank, magns.size(),
                                sampleRate, numFilters);
    }
    
    applyFilterBank(result, magns, _hzToMelFilterBank);
}

void
MelScale::melToHzFilter(vector<float> *result,
                        const vector<float> &magns,
                        float sampleRate, int numFilters)
{
    if ((magns.size() != _melToHzFilterBank._dataSize) ||
        (sampleRate != _melToHzFilterBank._sampleRate) ||
        (numFilters != _melToHzFilterBank._numFilters))
    {
        createFilterBankMelToHz(&_melToHzFilterBank, magns.size(),
                                sampleRate, numFilters);
    }
    
    applyFilterBank(result, magns, _melToHzFilterBank);
}

float
MelScale::computeTriangleAreaBetween(float txmin, float txmid, float txmax,
                                     float x0, float x1)
{
    if ((x0 > txmax) || (x1 < txmin))
        return 0.0;
    
    vector<float> &x = _tmpBuf0;
    x.resize(5);
    x[0] = txmin;
    x[1] = txmid;
    x[2] = txmax;
    x[3] = x0;
    x[4] = x1;
    
    sort(x.begin(), x.end());
    
    float points[5][2];
    for (int i = 0; i < 5; i++)
    {
        points[i][0] = x[i];
        points[i][1] = computeTriangleY(txmin, txmid, txmax, x[i]);
    }
    
    float area = 0.0;
    for (int i = 0; i < 4; i++)
    {
        // Suppress the cases which are out of [x0, x1] bounds
        if ((points[i][0] >= x1) ||
            (points[i + 1][0] <= x0))
            continue;
            
        float y0 = points[i][1];
        float y1 = points[i + 1][1];
        if (y0 > y1)
        {
            float tmp = y0;
            y0 = y1;
            y1 = tmp;
        }
        
        float a = (points[i + 1][0] - points[i][0])*(y0 + (y1 - y0)*0.5);
        
        area += a;
    }
    
    return area;
}

float
MelScale::computeTriangleY(float txmin, float txmid, float txmax,
                           float x)
{
    if (x <= txmin)
        return 0.0;
    if (x >= txmax)
        return 0.0;
    
    if (x <= txmid)
    {
        float t = (x - txmin)/(txmid - txmin);
        
        return t;
    }
    else // x >= txmid
    {
        float t = 1.0 - (x - txmid)/(txmax - txmid);
        
        return t;
    }
}

// See: https://haythamfayek.com/2016/04/21/speech-processing-for-machine-learning.html
void
MelScale::createFilterBankHzToMel(FilterBank *filterBank, int dataSize,
                                  float sampleRate, int numFilters)
{    
    filterBank->_dataSize = dataSize;
    filterBank->_sampleRate = sampleRate;
    filterBank->_numFilters = numFilters;
    
    filterBank->_filters.resize(filterBank->_numFilters);
    
    for (int i = 0; i < filterBank->_filters.size(); i++)
    {
        filterBank->_filters[i]._data.resize(dataSize);
        Utils::fillZero(&filterBank->_filters[i]._data);
        
        filterBank->_filters[i]._bounds[0] = -1;
        filterBank->_filters[i]._bounds[1] = -1;
    }
    
    // Create filters
    float lowFreqMel = 0.0;
    float highFreqMel = hzToMel(sampleRate*0.5);
    
    // Compute equally spaced mel values
    vector<float> melPoints;
    melPoints.resize(numFilters + 2);
    for (int i = 0; i < melPoints.size(); i++)
    {
        // Compute mel value
        float t = ((float)i)/(melPoints.size() - 1);
        float val = lowFreqMel + t*(highFreqMel - lowFreqMel);
        
        melPoints.data()[i] = val;
    }
    
    // Compute mel points
    vector<float> hzPoints;
    hzPoints.Resize(melPoints.size());
    for (int i = 0; i < hzPoints.size(); i++)
    {
        // Compute hz value
        float val = melPoints.data()[i];
        val = melToHz(val);
        
        hzPoints.data()[i] = val;
    }
    
    // Compute bin points
    vector<float> bin;
    bin.Resize(hzPoints.GetSize());
    
    float hzPerBinInv = (dataSize + 1)/(sampleRate*0.5);
    for (int i = 0; i < bin.size(); i++)
    {
        // Compute hz value
        float val = hzPoints.data()[i];
        
        // For the new solution that fills holes, do not round or trunk
        val = val*hzPerBinInv;
        
        bin.data()[i] = val;
    }
    
    // For each destination value
    for (int i = 0; i < dataSize; i++)
    {
        // For each filter
        for (int m = 1; m < numFilters + 1; m++)
        {
            float fmin = bin.data()[m - 1]; // left
            float fmid = bin.data()[m];     // center
            float fmax = bin.data()[m + 1]; // right
            
            //
            filterBank->_filters[m - 1]._bounds[0] = std::floor(fmin);
            filterBank->_filters[m - 1]._bounds[1] = std::ceil(fmax);
            
            // Trapezoid
            float x0 = i;
            if (fmin > x0)
                x0 = fmin;
            
            float x1 = i + 1;
            if (fmax < x1)
                x1 = fmax;
            
            float tarea = computeTriangleAreaBetween(fmin, fmid, fmax, x0, x1);
            
            // Normalize
            tarea /= (fmid - fmin)*0.5 + (fmax - fmid)*0.5;
            
            filterBank->_filters[m - 1]._data.data()[i] += tarea;
        }
    }
}

void
MelScale::createFilterBankMelToHz(FilterBank *filterBank, int dataSize,
                                  float sampleRate, int numFilters)
{    
    filterBank->_dataSize = dataSize;
    filterBank->_sampleRate = sampleRate;
    filterBank->_numFilters = numFilters;
    
    filterBank->_filters.resize(filterBank->_numFilters);
    
    for (int i = 0; i < filterBank->_filters.size(); i++)
    {
        filterBank->_filters[i]._data.resize(dataSize);
        Utils::fillZero(&filterBank->_filters[i].mData);
        
        filterBank->_filters[i]._bounds[0] = -1;
        filterBank->_filters[i]._bounds[1] = -1;
    }
    
    // Create filters
    //
    float lowFreqHz = 0.0;
    float highFreqHz = sampleRate*0.5;
    
    vector<float> hzPoints;
    hzPoints.resize(numFilters + 2);
    for (int i = 0; i < hzPoints.size(); i++)
    {
        // Compute hz value
        float t = ((float)i)/(hzPoints.size() - 1);
        float val = lowFreqHz + t*(highFreqHz - lowFreqHz);
        
        hzPoints.data()[i] = val;
    }
    
    // Compute hz points
    vector<float> melPoints;
    melPoints.resize(hzPoints.size());
    for (int i = 0; i < melPoints.size(); i++)
    {
        // Compute hz value
        float val = hzPoints.data()[i];
        val = hzToMel(val);
        melPoints.data()[i] = val;
    }
    
    // Compute bin points
    vector<float> bin;
    bin.resize(melPoints.size());
    
    float maxMel = hzToMel(sampleRate*0.5);
    float melPerBinInv = (dataSize + 1)/maxMel;
    for (int i = 0; i < bin.size(); i++)
    {
        // Compute mel value
        float val = melPoints.data()[i];
        
        // For the new solution that fills holes, do not round or trunk
        val = val*melPerBinInv;
        
        bin.data()[i] = val;
    }
    
    // For each destination value
    for (int i = 0; i < dataSize; i++)
    {
        // For each filter
        for (int m = 1; m < numFilters; m++)
        {
            float fmin = bin.data()[m - 1]; // left
            float fmid = bin.data()[m];     // center
            float fmax = bin.data()[m + 1]; // right
            
            //
            filterBank->_filters[m]._bounds[0] = std::floor(fmin);
            filterBank->_filters[m]._bounds[1] = std::ceil(fmax);
            
            // Trapezoid
            float x0 = i;
            if (fmin > x0)
                x0 = fmin;
            
            float x1 = i + 1;
            if (fmax < x1)
                x1 = fmax;
            
            float tarea = computeTriangleAreaBetween(fmin, fmid, fmax, x0, x1);
            
            // Normalize
            tarea /= (fmid - fmin)*0.5 + (fmax - fmid)*0.5;
            
            filterBank->_filters[m /*- 1*/]._data.data()[i] += tarea;
        }
    }
}

void
MelScale::applyFilterBank(vector<float> *result,
                          const vector<float> &magns,
                          const FilterBank &filterBank)
{
    result->resize(filterBank._numFilters);
    Utils::fillZero(result);
    
    // For each filter
    for (int m = 0; m < filterBank._numFilters; m++)
    {
        const FilterBank::Filter &filter = filterBank._filters[m];

        const float *filterData = filter._data.data();
        float *resultData = result->data();
        float *magnsData = magns.data();
        
        // For each destination value
        for (int i = filter._bounds[0] - 1; i < filter._bounds[1] + 1; i++)
        {
            if (i < 0)
                continue;
            if (i >= magns.size())
                continue;
            
            // Apply the filter value
            resultData[m] += filterData[i]*magnsData[i];
        }
    }
}
