#include <cmath>
#include <math.h> // for exp10

#include "Defines.h"
#include "MelScale.h"
#include "FilterBank.h"
#include "Utils.h"

#include "Scale.h"

#define LOG_SCALE2_FACTOR 3.5

// Center on 1000Hz
// NOTE: the value of 100 is a hack for the moment,
// with this, the center freq is between 1000 and 2000Hz
#define LOG_CENTER_FREQ 100.0

#define LOW_ZOOM_GAMMA 0.8

#define LOG_EPS 1e-35

Scale::Scale()
{
    for (int i = 0; i < NUM_FILTER_BANKS; i++)
        _filterBanks[i] = NULL;
}

Scale::~Scale()
{
    for (int i = 0; i < NUM_FILTER_BANKS; i++)
    {
        if (_filterBanks[i] != NULL)
            delete _filterBanks[i];
    }
}

float
Scale::applyScale(Type scaleType,
                  float x, float minValue, float maxValue)
{
    if (scaleType == NORMALIZED)
    {
        x = valueToNormalized(x, minValue, maxValue);
    }
    else if (scaleType == DB)
    {
        x = normalizedToDB(x, minValue, maxValue);
    }
    else if (scaleType == LOG)
    {
        x = normalizedToLog(x, minValue, maxValue);
    }
    else if (scaleType == LOG10)
    {
        x = normalizedToLog10(x, minValue, maxValue);
    }
    else if (scaleType == LOG_FACTOR)
    {
        x = normalizedToLogScale(x);
    }
    else if ((scaleType == MEL) || (scaleType == MEL_FILTER))
    {
        x = normalizedToMel(x, minValue, maxValue);
    }
    else if ((scaleType == MEL_INV) || (scaleType == MEL_FILTER_INV))
    {
        x = normalizedToMelInv(x, minValue, maxValue);
    }
    else if (scaleType == DB_INV)
    {
        x = normalizedToDBInv(x, minValue, maxValue);
    }
    else if (scaleType == LOW_ZOOM)
    {
        x = normalizedToLowZoom(x, minValue, maxValue);
    }
    else if (scaleType == LOG_NO_NORM)
    {
        x = toLog(x);
    }
    else if (scaleType == LOG_NO_NORM_INV)
    {
        x = toLogInv(x);
    }
    
    return x;
}

float
Scale::applyScaleInv(Type scaleType,
                     float x, float minValue, float maxValue)
{
    if (scaleType == NORMALIZED)
    {
        x = valueToNormalizedInv(x, minValue, maxValue);
    }
    else if (scaleType == DB)
    {
        x = normalizedToDBInv(x, minValue, maxValue);
    }
    else if (scaleType == LOG)
    {
        x = normalizedToLogInv(x, minValue, maxValue);
    }
    else if (scaleType == LOG10)
    {
        x = normalizedToLog10Inv(x, minValue, maxValue);
    }
#if 0
    else if (scaleType == LOG_FACTOR)
    {
        x = normalizedToLogFactorInv(x, minValue, maxValue);
    }
#endif
    else if (scaleType == LOG_FACTOR)
    {
        x = normalizedToLogScaleInv(x);
    }
    else if ((scaleType == MEL) || (scaleType == MEL_FILTER))
    {
        x = normalizedToMelInv(x, minValue, maxValue);
    }
    else if (scaleType == LOW_ZOOM)
    {
        x = normalizedToLowZoomInv(x, minValue, maxValue);
    }
    else if (scaleType == LOG_NO_NORM)
    {
        x = toLogInv(x);
    }
    
    return x;
}

void
Scale::applyScaleForEach(Type scaleType,
                         vector<float> *values,
                         float minValue,
                         float maxValue)
{
    if (scaleType == NORMALIZED)
    {
        valueToNormalizedForEach(values, minValue, maxValue);
    }
    else if (scaleType == DB)
    {
        normalizedToDBForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOG)
    {
        normalizedToLogForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOG10)
    {
        normalizedToLog10ForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOG_FACTOR)
    {
        normalizedToLogScaleForEach(values);
    }
    else if ((scaleType == MEL) || (scaleType == MEL_FILTER))
    {
        normalizedToMelForEach(values, minValue, maxValue);
    }
    else if ((scaleType == MEL_INV) || (scaleType == MEL_FILTER_INV))
    {
        normalizedToMelInvForEach(values, minValue, maxValue);
    }
    else if (scaleType == DB_INV)
    {
        normalizedToDBInvForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOW_ZOOM)
    {
        normalizedToLowZoomForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOG_NO_NORM)
    {
        toLogForEach(values);
    }
    else if (scaleType == LOG_NO_NORM_INV)
    {
        toLogInvForEach(values);
    }
}
    
void
Scale::applyScaleInvForEach(Type scaleType,
                            vector<float> *values,
                            float minValue,
                            float maxValue)
{
    if (scaleType == NORMALIZED)
    {
        valueToNormalizedInvForEach(values, minValue, maxValue);
    }
    else if (scaleType == DB)
    {
        normalizedToDBInvForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOG)
    {
        normalizedToLogInvForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOG10)
    {
        normalizedToLog10InvForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOG_FACTOR)
    {
        normalizedToLogScaleInvForEach(values);
    }
    else if ((scaleType == MEL) || (scaleType == MEL_FILTER))
    {
        normalizedToMelInvForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOW_ZOOM)
    {
        normalizedToLowZoomInvForEach(values, minValue, maxValue);
    }
    else if (scaleType == LOG_NO_NORM)
    {
        toLogInvForEach(values);
    }
}

void
Scale::applyScale(Type scaleType,
                  vector<float> *values,
                  float minValue, float maxValue)
{
    if (scaleType == LOG_FACTOR)
    {
        dataToLogScale(values);
    }
    else if (scaleType == MEL)
    {
        dataToMel(values, minValue, maxValue);
    }
}

void
Scale::applyScaleFilterBank(FilterBankType fbType,
                            vector<float> *result,
                            const vector<float> &magns,
                            float sampleRate, int numFilters)
{
    if (fbType == FILTER_BANK_LINEAR)
    {
        // Try to not apply filter bank
        // Because even in linear, it modifies the data a little

        if (magns.size() == numFilters)
            // Size is the same, nothing to do, just copy
        {
            *result = magns;
            
            return;
        }

        // Otherwise, will need filter bank to resize the data
    }
    
    if (_filterBanks[(int)fbType] == NULL)
    {
        Type type = filterBankTypeToType(fbType);
        _filterBanks[(int)fbType] = new FilterBank(type);
    }
    
    _filterBanks[(int)fbType]->hzToTarget(result, magns, sampleRate, numFilters);    
}

void
Scale::applyScaleFilterBankInv(FilterBankType fbType,
                               vector<float> *result,
                               const vector<float> &magns,
                               float sampleRate, int numFilters)
{
    if (fbType == FILTER_BANK_LINEAR)
    {
        // Try to not apply filter bank
        // Because even in linear, it modifies the data a little

        if (magns.size() == numFilters)
            // Size is the same, nothing to do, just copy
        {
            *result = magns;
            
            return;
        }

        // Otherwise, will need filter bank to resize the data
    }
    
    if (_filterBanks[(int)fbType] == NULL)
    {
        Type type = filterBankTypeToType(fbType);
        
        _filterBanks[(int)fbType] = new FilterBank(type);
    }
    
    _filterBanks[(int)fbType]->targetToHz(result, magns, sampleRate, numFilters);
}

Scale::FilterBankType
Scale::typeToFilterBankType(Type type)
{
    switch (type)
    {
        case LINEAR:
            return FILTER_BANK_LINEAR;
            break;

        case LOG:
            return FILTER_BANK_LOG;
            break;
            
        case LOG10:
            return FILTER_BANK_LOG10;
            break;

        case LOG_FACTOR:
            return FILTER_BANK_LOG_FACTOR;
            break;

        case MEL:
        case MEL_FILTER:
            return FILTER_BANK_MEL;
            break;

        case LOW_ZOOM:
            return FILTER_BANK_LOW_ZOOM;
            break;
            
        default:
            return (FilterBankType)-1;
    }
}

Scale::Type
Scale::filterBankTypeToType(FilterBankType fbType)
{
    switch (fbType)
    {
        case FILTER_BANK_LINEAR:
            return LINEAR;
            break;

        case FILTER_BANK_LOG:
            return LOG;
            break;
            
        case FILTER_BANK_LOG10:
            return LOG10;
            break;

        case FILTER_BANK_LOG_FACTOR:
            return LOG_FACTOR;
            break;

        case FILTER_BANK_MEL:
            return MEL;
            break;

        case FILTER_BANK_LOW_ZOOM:
            return LOW_ZOOM;
            break;
            
        default:
            return (Type)-1;
    }
}

float
Scale::valueToNormalized(float y,
                         float minValue,
                         float maxValue)
{
    y = (y - minValue)/(maxValue - minValue);

    return y;
}

float
Scale::valueToNormalizedInv(float y,
                            float minValue,
                            float maxValue)
{
    y = y*(maxValue - minValue) + minValue;

    return y;
}

float
Scale::normalizedToDB(float x, float mindB, float maxdB)
{
    if (std::fabs(x) < NL_EPS)
        x = mindB;
    else
        x = Utils::ampToDB(x);
    
    x = (x - mindB)/(maxdB - mindB);
    
    // Avoid negative values, for very low x dB
    if (x < 0.0)
        x = 0.0;
    
    return x;
}

float
Scale::normalizedToDBInv(float x, float mindB, float maxdB)
{
    x = mindB + x*(maxdB - mindB);
    
    x = Utils::DBToAmp(x);
 
    // Avoid negative values, for very low x dB
    if (x < 0.0)
        x = 0.0;
    
    return x;
}

float
Scale::normalizedToLog10(float x, float minValue, float maxValue)
{
    x = x*(maxValue - minValue) + minValue;
    
    x = log10(1.0 + x);
    
    float lMin = log10(1.0 + minValue);
    float lMax = log10(1.0 + maxValue);
    
    x = (x - lMin)/(lMax - lMin);
    
    return x;
}

float
Scale::normalizedToLog10Inv(float x, float minValue, float maxValue)
{
    float lMin = log10(1.0 + minValue);
    float lMax = log10(1.0 + maxValue);
    
    x = x*(lMax - lMin) + lMin;
    
    x = pow((float)10.0, x) - 1.0;
    
    x = (x - minValue)/(maxValue - minValue);
    
    return x;
}

float
Scale::normalizedToLog(float x, float minValue, float maxValue)
{
    float a = (pow(10.0, 0.5) - 1.0)/LOG_CENTER_FREQ;
 
    float lMin = log10(1.0 + minValue*a);
    float lMax = log10(1.0 + maxValue*a);
    
    x = x*(maxValue - minValue) + minValue;
    
    x = log10(1.0 + x*a);
    
    x = (x - lMin)/(lMax - lMin);
    
    return x;
}

float
Scale::normalizedToLogInv(float x, float minValue, float maxValue)
{
    float a = (pow(10.0, 0.5) - 1.0)/LOG_CENTER_FREQ;
    
    float lMin = log10(1.0 + minValue*a);
    float lMax = log10(1.0 + maxValue*a);
    
    x = x*(lMax - lMin) + lMin;
    
    x = (pow((float)10.0, x) - 1.0)/a;
    
    x = (x - minValue)/(maxValue - minValue);
    
    return x;
}

float
Scale::normalizedToLogScale(float value)
{
    float t0 = log((float)1.0 + value*(exp(LOG_SCALE2_FACTOR) - 1.0))/LOG_SCALE2_FACTOR;
    
    return t0;
}

float
Scale::normalizedToLogScaleInv(float value)
{
    (((std::exp(LOG_SCALE2_FACTOR) - 1.0))/LOG_SCALE2_FACTOR);
    float t0 = (exp(value*LOG_SCALE2_FACTOR) - 1.0)/
        exp(LOG_SCALE2_FACTOR - 1.0);
    
    return t0;
}

float
Scale::normalizedToLowZoom(float x, float minValue, float maxValue)
{
    // 2 times mel
    float result = normalizedToMel(x, minValue, maxValue);
    result = Utils::applyGamma(result, (float)LOW_ZOOM_GAMMA); // mel + gamma

    return result;
}
    
float
Scale::normalizedToLowZoomInv(float x, float minValue, float maxValue)
{
    // 2 times mel inv
    float result =
        Utils::applyGamma(x, (float)(1.0 - LOW_ZOOM_GAMMA)); // mel + gamma
    result = normalizedToMelInv(result, minValue, maxValue);
        
    return result;
}

void
Scale::dataToLogScale(vector<float> *values)
{
    vector<float> &origValues = _tmpBuf0;
    origValues = *values;
    
    int valuesSize = values->size();
    float *valuesData = values->data();
    float *origValuesData = origValues.data();
    
    for (int i = 0; i < valuesSize; i++)
    {
        float t0 = ((float)i)/valuesSize;
        
        // "Inverse" process for data
        t0 *= LOG_SCALE2_FACTOR;
        float t =
            (exp(t0) - 1.0)/(exp(LOG_SCALE2_FACTOR) - 1.0);
        
        int dstIdx = (int)(t*valuesSize);
        
        if (dstIdx < 0)
            // Should not happen
            dstIdx = 0;
        
        if (dstIdx > valuesSize - 1)
            // We never know...
            dstIdx = valuesSize - 1;
        
        float dstVal = origValuesData[dstIdx];
        valuesData[i] = dstVal;
    }
}

float
Scale::normalizedToMel(float x,
                       float minFreq,
                       float maxFreq)
{
    x = x*(maxFreq - minFreq) + minFreq;
    
    x = MelScale::hzToMel(x);
    
    float lMin = MelScale::hzToMel(minFreq);
    float lMax = MelScale::hzToMel(maxFreq);
    
    x = (x - lMin)/(lMax - lMin);
    
    return x;
}

float
Scale::normalizedToMelInv(float x,
                          float minFreq,
                          float maxFreq)
{
    float minMel = MelScale::hzToMel(minFreq);
    float maxMel = MelScale::hzToMel(maxFreq);
    
    x = x*(maxMel - minMel) + minMel;
    
    x = MelScale::melToHz(x);
    
    x = (x - minFreq)/(maxFreq - minFreq);
    
    return x;
}

float
Scale::toLog(float x)
{
    x = log(x + LOG_EPS);

    return x;
}
    
float
Scale::toLogInv(float x)
{
    x = exp(x);

    return x;
}

void
Scale::dataToMel(vector<float> *values,
                 float minFreq, float maxFreq)
{
    vector<float> &origValues = _tmpBuf1;
    origValues = *values;
    
    int valuesSize = values->size();
    float *valuesData = values->data();
    float *origValuesData = origValues.data();
    
    float minMel = MelScale::hzToMel(minFreq);
    float maxMel = MelScale::hzToMel(maxFreq);

    float t0 = 0.0;
    float t0incr = 1.0/valuesSize;

    float freqCoeff = 1.0/(maxFreq - minFreq);
    for (int i = 0; i < valuesSize; i++)
    {
        // "Inverse" process for data
        float mel = minMel + t0*(maxMel - minMel);
        float freq = MelScale::melToHz(mel);
        float t = (freq - minFreq)*freqCoeff;

        int dstIdx = (int)(t*valuesSize);
        
        if (dstIdx < 0)
            // Should not happen
            dstIdx = 0;
        
        if (dstIdx > valuesSize - 1)
            // We never know...
            dstIdx = valuesSize - 1;
        
        float dstVal = origValuesData[dstIdx];
        valuesData[i] = dstVal;

        t0 += t0incr;
    }
}

void
Scale::valueToNormalizedForEach(vector<float> *values,
                                float minValue, float maxValue)
{
    int numValues = values->size();
    float *valuesData = values->data();

    float coeffInv = 0.0;
    if (maxValue - minValue > NL_EPS)
        coeffInv = 1.0/(maxValue - minValue);
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];

        x = (x - minValue)*coeffInv;
        
         valuesData[i] = x;
    }
}
    
void
Scale::valueToNormalizedInvForEach(vector<float> *values,
                                   float minValue, float maxValue)
{
    int numValues = values->size();
    float *valuesData = values->data();

    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];

        x = x*(maxValue - minValue) + minValue;
        
        valuesData[i] = x;
    }
}

void
Scale::normalizedToDBForEach(vector<float> *values,
                             float mindB,
                             float maxdB)
{
    int numValues = values->size();
    float *valuesData = values->data();

    float coeffInv = 0.0;
    if (maxdB - mindB > NL_EPS)
        coeffInv = 1.0/(maxdB - mindB);
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        if (std::fabs(x) < NL_EPS)
            x = mindB;
        else
            x = Utils::ampToDB(x);
        
        x = (x - mindB)*coeffInv;
        
        // Avoid negative values, for very low x dB
        if (x < 0.0)
            x = 0.0;

        valuesData[i] = x;
    }
}

void
Scale::normalizedToDBInvForEach(vector<float> *values,
                                float mindB,
                                float maxdB)
{
    int numValues = values->size();
    float *valuesData = values->data();

    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        x = mindB + x*(maxdB - mindB);
        
        x = Utils::DBToAmp(x);
        
        // Avoid negative values, for very low x dB
        if (x < 0.0)
            x = 0.0;

        valuesData[i] = x;
    }
}
    
void
Scale::normalizedToLog10ForEach(vector<float> *values,
                                float minValue,
                                float maxValue)
{
    int numValues = values->size();
    float *valuesData = values->data();

    float lMin = log10(1.0 + minValue);
    float lMax = log10(1.0 + maxValue);
        
    float coeffInv = 0.0;
    if (lMax - lMin > NL_EPS)
        coeffInv = 1.0/(lMax - lMin);
                        
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        x = x*(maxValue - minValue) + minValue;
    
        x = log10(1.0 + x);
    
        x = (x - lMin)*coeffInv;

        valuesData[i] = x;
    }
}
    
void
Scale::normalizedToLog10InvForEach(vector<float> *values,
                                   float minValue,
                                   float maxValue)
{
    int numValues = values->size();
    float *valuesData = values->data();

    float lMin = log10(1.0 + minValue);
    float lMax = log10(1.0 + maxValue);
        
    float coeffInv = 0.0;
    if (maxValue - minValue > NL_EPS)
        coeffInv = 1.0/(maxValue - minValue);
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        x = x*(lMax - lMin) + lMin;
    
        x = pow((float)10.0, x) - 1.0;
    
        x = (x - minValue)*coeffInv;

        valuesData[i] = x;
    }
}

void
Scale::normalizedToLogForEach(vector<float> *values,
                              float minValue,
                              float maxValue)
{
    float a = (pow(10.0, 0.5) - 1.0)/LOG_CENTER_FREQ;
    
    int numValues = values->size();
    float *valuesData = values->data();

    float lMin = log10(1.0 + minValue*a);
    float lMax = log10(1.0 + maxValue*a);
        
    float coeffInv = 0.0;
    if (lMax - lMin > NL_EPS)
        coeffInv = 1.0/(lMax - lMin);
                        
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        x = x*(maxValue - minValue) + minValue;
    
        x = log10(1.0 + x*a);
    
        x = (x - lMin)*coeffInv;

        valuesData[i] = x;
    }
}
    
void
Scale::normalizedToLogInvForEach(vector<float> *values,
                                 float minValue,
                                 float maxValue)
{
    float a = (pow(10.0, 0.5) - 1.0)/LOG_CENTER_FREQ;
    
    int numValues = values->size();
    float *valuesData = values->data();

    float lMin = log10(1.0 + minValue*a);
    float lMax = log10(1.0 + maxValue*a);
        
    float coeffInv = 0.0;
    if (maxValue - minValue > NL_EPS)
        coeffInv = 1.0/(maxValue - minValue);
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        x = x*(lMax - lMin) + lMin;
    
        x = (pow((float)10.0, x) - 1.0)/a;
    
        x = (x - minValue)*coeffInv;

        valuesData[i] = x;
    }
}

void
Scale::normalizedToLogScaleForEach(vector<float> *values)
{
    int numValues = values->size();
    float *valuesData = values->data();

    float coeffInv = 1.0/LOG_SCALE2_FACTOR;
    float coeff2 = exp(LOG_SCALE2_FACTOR) - 1.0;
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        x = log((float)1.0 + x*coeff2)*coeffInv;
        valuesData[i] = x;
    }
}
    
void
Scale::normalizedToLogScaleInvForEach(vector<float> *values)
{
    int numValues = values->size();
    float *valuesData = values->data();

    float coeffInv = 1.0/exp(LOG_SCALE2_FACTOR - 1.0);
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        x = (exp(x*LOG_SCALE2_FACTOR) - 1.0)*coeffInv;
        
        valuesData[i] = x;
    }
}
    
void
Scale::normalizedToMelForEach(vector<float> *values,
                              float minFreq,
                              float maxFreq)
{
    int numValues = values->size();
    float *valuesData = values->data();

    float lMin = MelScale::hzToMel(minFreq);
    float lMax = MelScale::hzToMel(maxFreq);
        
    float coeffInv = 0.0;
    if (lMax - lMin > NL_EPS)
        coeffInv = 1.0/(lMax - lMin);
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        x = x*(maxFreq - minFreq) + minFreq;
    
        x = MelScale::hzToMel(x);
    
        x = (x - lMin)*coeffInv;;

        valuesData[i] = x;
    }
}
    
void
Scale::normalizedToMelInvForEach(vector<float> *values,
                                 float minFreq,
                                 float maxFreq)
{
    int numValues = values->size();
    float *valuesData = values->data();

    float minMel = MelScale::hzToMel(minFreq);
    float maxMel = MelScale::hzToMel(maxFreq);
        
    float coeffInv = 0.0;
    if (maxFreq - minFreq > NL_EPS)
        coeffInv = 1.0/(maxFreq - minFreq);
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
    
        x = x*(maxMel - minMel) + minMel;
        
        x = MelScale::melToHz(x);
    
        x = (x - minFreq)*coeffInv;
        
        valuesData[i] = x;
    }
}

void
Scale::normalizedToLowZoomForEach(vector<float> *values,
                                  float minFreq, float maxFreq)
{
    // 2 times mel
    int numValues = values->size();
    float *valuesData = values->data();

    float lMin = MelScale::hzToMel(minFreq);    
    float lMax = MelScale::hzToMel(maxFreq);
        
    float coeffInv = 0.0;
    if (lMax - lMin > NL_EPS)
        coeffInv = 1.0/(lMax - lMin);
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];
        
        x = x*(maxFreq - minFreq) + minFreq;
    
        x = MelScale::hzToMel(x);
        
        x = (x - lMin)*coeffInv;;

        x = Utils::applyGamma(x, (float)LOW_ZOOM_GAMMA);
        
        valuesData[i] = x;
    }
}
    
void
Scale::normalizedToLowZoomInvForEach(vector<float> *values,
                                     float minValue, float maxValue)
{
    // 2 times mel inv
    int numValues = values->size();
    float *valuesData = values->data();

    float minMel = MelScale::hzToMel(minValue);    
    float maxMel = MelScale::hzToMel(maxValue);
        
    float coeffInv = 0.0;
    if (maxValue - minValue > NL_EPS)
        coeffInv = 1.0/(maxValue - minValue);
    
    for (int i = 0; i < numValues; i++)
    {
        float x = valuesData[i];

        x = Utils::applyGamma(x, (float)(1.0 - LOW_ZOOM_GAMMA));
        
        x = x*(maxMel - minMel) + minMel;
        
        x = MelScale::melToHz(x);
    
        x = (x - minValue)*coeffInv;
        
        valuesData[i] = x;
    }
}

void
Scale::toLogForEach(vector<float> *values)
{
    for (int i = 0; i < values->size(); i++)
    {
        float x = values->data()[i];
        x = log(x + LOG_EPS);
        values->data()[i] = x;
    }
}
    
void
Scale::toLogInvForEach(vector<float> *values)
{
    for (int i = 0; i < values->size(); i++)
    {
        float x = values->data()[i];
        x = exp(x);
        values->data()[i] = x;
    }
}
