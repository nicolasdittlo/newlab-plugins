#include <algorithm>
using namespace std;

#include "Utils.h"

#include "FilterBank.h"

#define FIX_ALIASING_MIN_TRIANGLE_WIDTH 2.0

// Filter bank
FilterBank::FilterBankObj::FilterBankObj(int dataSize,
                                         float sampleRate,
                                         int numFilters)
{
    _dataSize = dataSize;
    _sampleRate = sampleRate;
    _numFilters = numFilters;
    
    _filters.resize(_numFilters);
    for (int i = 0; i < _filters.size(); i++)
    {
        _filters[i]._data.resize(dataSize);
        Utils::fillZero(&_filters[i]._data);
        
        _filters[i]._bounds[0] = -1;
        _filters[i]._bounds[1] = -1;
    }
}

FilterBank::FilterBankObj::FilterBankObj()
{
    _dataSize = 0;
    _sampleRate = 0.0;
    _numFilters = 0;
}

FilterBank::FilterBankObj::~FilterBankObj() {}

// See: http://practicalcryptography.com/miscellaneous/machine-learning/guide-mel-frequency-cepstral-coefficients-mfccs/
FilterBank::FilterBank(Scale::Type targetScaleType)
{
    _targetScaleType = targetScaleType;

    _scale = new Scale();
}

FilterBank::~FilterBank()
{
    delete _scale;
}

void
FilterBank::hzToTarget(vector<float> *result,
                       const vector<float> &magns,
                       float sampleRate, int numFilters)
{
    if ((magns.size() != _hzToTargetFilterBank._dataSize) ||
        (sampleRate != _hzToTargetFilterBank._sampleRate) ||
        (numFilters != _hzToTargetFilterBank._numFilters))
    {
        createFilterBankHzToTarget(&_hzToTargetFilterBank, magns.size(),
                                   sampleRate, numFilters);
    }
    
    applyFilterBank(result, magns, _hzToTargetFilterBank);
}

void
FilterBank::targetToHz(vector<float> *result,
                       const vector<float> &magns,
                       float sampleRate, int numFilters)
{
    if ((magns.size() != _targetToHzFilterBank._dataSize) ||
        (sampleRate != _targetToHzFilterBank._sampleRate) ||
        (numFilters != _targetToHzFilterBank._numFilters))
    {
        createFilterBankTargetToHz(&_targetToHzFilterBank, magns.size(),
                                   sampleRate, numFilters);
    }
    
    applyFilterBank(result, magns, _targetToHzFilterBank);
}

float
FilterBank::computeTriangleAreaBetween(float txmin, float txmid, float txmax,
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
FilterBank::computeTriangleY(float txmin, float txmid, float txmax,
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
FilterBank::createFilterBankHzToTarget(FilterBankObj *filterBank, int dataSize,
                                       float sampleRate, int numFilters)
{
    // Clear previous
    filterBank->_filters.resize(0);

    // Init
    filterBank->_dataSize = dataSize;
    filterBank->_sampleRate = sampleRate;
    filterBank->_numFilters = numFilters;
    
    filterBank->_filters.resize(filterBank->_numFilters);
    
    for (int i = 0; i < filterBank->mFilters.size(); i++)
    {
        filterBank->_filters[i]._data.resize(dataSize);
        Utils::fillZero(&filterBank->_filters[i]._data);
        
        filterBank->_filters[i]._bounds[0] = -1;
        filterBank->_filters[i]._bounds[1] = -1;
    }
    
    // Create filters
    //
    float lowFreqTarget = 0.0;
    float highFreqTarget = applyScale(sampleRate*0.5, 0.0, sampleRate*0.5);
    
    // Compute equally spaced target values
    vector<float> targetPoints;
    targetPoints.resize(numFilters + 2);
    for (int i = 0; i < targetPoints.size(); i++)
    {
        // Compute target value
        float t = ((float)i)/(targetPoints.size() - 1);
        float val = lowFreqTarget + t*(highFreqTarget - lowFreqTarget);
        
        targetPoints.data()[i] = val;
    }
    
    // Compute target points
    vector<float> hzPoints;
    hzPoints.resize(targetPoints.size());
    for (int i = 0; i < hzPoints.size(); i++)
    {
        // Compute hz value
        float val = targetPoints.data()[i];
        val = applyScaleInv(val, 0.0, sampleRate*0.5);
        
        hzPoints.data()[i] = val;
    }
    
    // Compute bin points
    vector<float> bin;
    bin.resize(hzPoints.size());
    
    float hzPerBinInv = (dataSize + 1)/(sampleRate*0.5);
    for (int i = 0; i < bin.size(); i++)
    {
        // Compute hz value
        float val = hzPoints.data()[i];
        
        // For the new solution that fills holes, do not round or trunk
        val = val*hzPerBinInv;
        
        bin.data()[i] = val;
    }
    
    // For each filter
    for (int m = 1; m < numFilters + 1; m++)
    {
        float fmin = bin.data()[m - 1]; // left
        float fmid = bin.data()[m];     // center
        float fmax = bin.data()[m + 1]; // right

        fixSmallTriangles(&fmin, &fmax, dataSize);
            
        filterBank->_filters[m - 1].mBounds[0] = std::floor(fmin);
        filterBank->_filters[m - 1].mBounds[1] = std::ceil(fmax);

        // Check upper bound
        if (filterBank->_filters[m - 1]._bounds[1] > dataSize - 1)
            filterBank->_filters[m - 1]._bounds[1] = dataSize - 1;
        
        for (int i = filterBank->_filters[m - 1]._bounds[0];
             i <= filterBank->_filters[m - 1]._bounds[1]; i++)
        {
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
            
            filterBank->_filters[m - 1].data.data()[i] += tarea;
        }
    }
}

void
FilterBank::createFilterBankTargetToHz(FilterBankObj *filterBank, int dataSize,
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
    vector<float> targetPoints;
    targetPoints.resize(hzPoints.size());
    for (int i = 0; i < targetPoints.size(); i++)
    {
        // Compute hz value
        float val = hzPoints.data()[i];
        val = applyScale(val, 0.0, sampleRate*0.5);
        
        targetPoints.data()[i] = val;
    }
    
    // Compute bin points
    vector<float> bin;
    bin.resize(targetPoints.size());
    
    float maxTarget = applyScale(sampleRate*0.5, 0.0, sampleRate*0.5);
    float targetPerBinInv = (dataSize + 1)/maxTarget;
    for (int i = 0; i < bin.size(); i++)
    {
        // Compute target value
        float val = targetPoints.data()[i];
        
        // For the new solution that fills holes, do not round or trunk
        val = val*targetPerBinInv;
        
        bin.data()[i] = val;
    }
    
    // For each filter
    for (int m = 1; m < numFilters; m++)
    {
        float fmin = bin.data()[m - 1]; // left
        float fmid = bin.data()[m];     // center
        float fmax = bin.data()[m + 1]; // right

        fixSmallTriangles(&fmin, &fmax, dataSize);
            
        //
        filterBank->_filters[m]._bounds[0] = std::floor(fmin);
        filterBank->_filters[m]._bounds[1] = std::ceil(fmax);

        // Check upper bound
        if (filterBank->_filters[m ]._bounds[1] > dataSize - 1)
            filterBank->_filters[m]._bounds[1] = dataSize - 1;
        
        for (int i = filterBank->_filters[m]._bounds[0];
             i <= filterBank->_filters[m]._bounds[1]; i++)
        {
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
            
            filterBank->_filters[m]._data.data()[i] += tarea;
        }
    }
}

void
FilterBank::applyFilterBank(vector<float> *result,
                            const vector<float> &magns,
                            const FilterBankObj &filterBank)
{
    result->resize(filterBank._numFilters);
    Utils::fillZero(result);
    
    // For each filter
    for (int m = 0; m < filterBank._numFilters; m++)
    {
        const FilterBankObj::Filter &filter = filterBank._filters[m];

        const float *filterData = filter._data.data();
        float *resultData = result->data();
        float *magnsData = magns.data();
        
        // For each destination value
        for (int i = filter._bounds[0]; i <= filter._bounds[1]; i++)
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

float
FilterBank::applyScale(float val, float minFreq, float maxFreq)
{
    float minTarget = _scale->applyScale(_targetScaleType, 0.0, minFreq, maxFreq);
    float maxTarget = _scale->applyScale(_targetScaleType, 1.0, minFreq, maxFreq);;

    // Normalize
    val = (val - minFreq)/(maxFreq - minFreq);
    
    // Apply (normalized)
    val = _scale->applyScale(_targetScaleType, val, minFreq, maxFreq);

    // De normalize
    val = val*(maxTarget - minTarget) + minTarget;
    
    return val;
}

float
FilterBank::applyScaleInv(float val, float minFreq, float maxFreq)
{
    float minTarget = _scale->applyScale(_targetScaleType, 0.0, minFreq, maxFreq);
    float maxTarget = _scale->applyScale(_targetScaleType, 1.0, minFreq, maxFreq);

    // Normalized
    val = (val - minTarget)/(maxTarget - minTarget);
 
    // Apply (normalized)
    val = _scale->applyScaleInv(_targetScaleType, val, minFreq, maxFreq);

    // De normalize
    val = val*(maxFreq - minFreq) + minFreq;
    
    return val;
}

void
FilterBank::fixSmallTriangles(float *fmin, float *fmax, int dataSize)
{
    if (dataSize < FIX_ALIASING_MIN_TRIANGLE_WIDTH)
        return;
    
    if (*fmax - *fmin < FIX_ALIASING_MIN_TRIANGLE_WIDTH)
    {
        float diff = FIX_ALIASING_MIN_TRIANGLE_WIDTH - (*fmax - *fmin);
        *fmin -= diff*0.5;
        *fmax += diff*0.5;

        if (*fmin < 0.0)
        {
            *fmax += -*fmin;
            *fmin = 0.0;
        }

        if (*fmax > dataSize - 1)
        {
            *fmin -= *fmax - (dataSize - 1);
            *fmax = dataSize - 1;
        }
    }
}
