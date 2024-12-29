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

#ifndef SCALE_H
#define SCALE_H

#include <vector>
using namespace std;

class MelScale;
class FilterBank;
class Scale
{
public:
    // Note: log10, or log-anything is the same since we use normalized values
    enum Type
    {
        LINEAR = 0,
        NORMALIZED,
        DB,
        LOG,
        LOG10,
        LOG_FACTOR,
        MEL, // Quick Mel
        MEL_FILTER, // Mel with real filters,
        MEL_INV,
        MEL_FILTER_INV,
        DB_INV,
        LOW_ZOOM, // Zoom on low freqs
        LOG_NO_NORM, // Log, but without any normalization
        LOG_NO_NORM_INV
    };

    // Filter banks
    enum FilterBankType
    {
        FILTER_BANK_LINEAR = 0,
        FILTER_BANK_LOG,
        FILTER_BANK_LOG10,
        FILTER_BANK_LOG_FACTOR,
        FILTER_BANK_MEL,
        FILTER_BANK_LOW_ZOOM,
        NUM_FILTER_BANKS
    };
     
    Scale();
    virtual ~Scale();

    // Apply to Y

    // Apply value by value
    
    // Generic
    float applyScale(Type scaleType,
                     float x,
                     float minValue = -1.0,
                     float maxValue = -1.0);
    
    float applyScaleInv(Type scaleType,
                        float x,
                        float minValue = -1.0,
                        float maxValue = -1.0);

    // Apply for each value
    void applyScaleForEach(Type scaleType,
                           vector<float> *values,
                           float minValue = -1.0,
                           float maxValue = -1.0);
    
    void applyScaleInvForEach(Type scaleType,
                              vector<float> *values,
                              float minValue = -1.0,
                              float maxValue = -1.0);
    
    // Apply to X
    
    void applyScale(Type scaleType,
                    vector<float> *values,
                    float minValue = -1.0,
                    float maxValue = -1.0);

    // Filter banks
    //
    // Can decimate or increase the data size
    // as the same time as scaling!
    
    void applyScaleFilterBank(FilterBankType type,
                              vector<float> *result,
                              const vector<float> &magns,
                              float sampleRate, int numFilters);

    void applyScaleFilterBankInv(FilterBankType type,
                                 vector<float> *result,
                                 const vector<float> &magns,
                                 float sampleRate, int numFilters);

    FilterBankType typeToFilterBankType(Type type);
    Type filterBankTypeToType(FilterBankType fbType);
    
protected:
    float valueToNormalized(float y,
                            float minValue,
                            float maxValue);

    float valueToNormalizedInv(float y,
                               float minValue,
                               float maxValue);
    
    float normalizedToDB(float y, float mindB,
                         float maxdB);
    
    float normalizedToDBInv(float y, float mindB,
                            float maxdB);
    
    float normalizedToLog10(float x, float minValue,
                            float maxValue);
    
    float normalizedToLog10Inv(float x, float minValue,
                               float maxValue);

    float normalizedToLog(float x, float minValue,
                          float maxValue);
    
    float normalizedToLogInv(float x, float minValue,
                                float maxValue);
    
    // Apply to axis for example
    float normalizedToLogScale(float value);
    
    float normalizedToLogScaleInv(float value);

    float normalizedToLowZoom(float x, float minValue,
                              float maxValue);
    
    float normalizedToLowZoomInv(float x, float minValue,
                                    float maxValue);
    
    void dataToLogScale(vector<float> *values);
    
    float normalizedToMel(float x,
                          float minFreq,
                          float maxFreq);
    
    float normalizedToMelInv(float x,
                             float minFreq,
                             float maxFreq);

    float toLog(float x);
    
    float toLogInv(float x);
    
    void dataToMel(vector<float> *values,
                   float minFreq, float maxFreq);

    // Process in block
    
    void valueToNormalizedForEach(vector<float> *values,
                                  float minValue, float maxValue);
    
    void valueToNormalizedInvForEach(vector<float> *values,
                                     float minValue, float maxValue);
    
    void normalizedToDBForEach(vector<float> *values,
                               float mindB, float maxdB);

    void normalizedToDBInvForEach(vector<float> *values,
                                  float mindB, float maxdB);    
 
    void normalizedToLog10ForEach(vector<float> *values,
                                  float minValue, float maxValue);
    
    void normalizedToLog10InvForEach(vector<float> *values,
                                     float minValue, float maxValue);

    void normalizedToLogForEach(vector<float> *values,
                                float minValue, float maxValue);
    
    void normalizedToLogInvForEach(vector<float> *values,
                                   float minValue, float maxValue);
    
    void normalizedToLogScaleForEach(vector<float> *values);
    
    void normalizedToLogScaleInvForEach(vector<float> *values);
    
    void normalizedToMelForEach(vector<float> *values,
                                float minFreq, float maxFreq);
    
    void normalizedToMelInvForEach(vector<float> *values,
                                   float minFreq, float maxFreq);

    void normalizedToLowZoomForEach(vector<float> *values,
                                    float minValue, float maxValue);
    
    void normalizedToLowZoomInvForEach(vector<float> *values,
                                       float minValue, float maxValue);

    void toLogForEach(vector<float> *values);
    
    void toLogInvForEach(vector<float> *values);
    
    FilterBank *_filterBanks[NUM_FILTER_BANKS];
    
private:
    // Tmp buffers
    vector<float> _tmpBuf0;
    vector<float> _tmpBuf1;
    vector<float> _tmpBuf2;
    vector<float> _tmpBuf3;
};

#endif
