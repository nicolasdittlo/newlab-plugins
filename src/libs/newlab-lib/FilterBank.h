#ifndef FILTER_BANK_H
#define FILTER_BANK_H

#include <vector>
using namespace std;

#include <Scale.h>


// From MelScale
// But generalized to arbitrary scale type, not only Mel

class FilterBank
{
public:
    FilterBank(Scale::Type targetScaleType);
    virtual ~FilterBank();

    // Can decimate or increase the data size
    // as the same time as scaling!
    
    // Use real Hz to Mel conversion, using filters
    void hzToTarget(vector<float> *result,
                    const vector<float> &magns,
                    float sampleRate, int numFilters);
    // Inverse
    void targetToHz(vector<float> *result,
                    const vector<float> &magns,
                    float sampleRate, int numFilters);
    
protected:
    float computeTriangleAreaBetween(float txmin,
                                     float txmid,
                                     float txmax,
                                     float x0, float x1);
    static float computeTriangleY(float txmin, float txmid, float txmax,
                                  float x);

    float applyScale(float val, float minFreq, float maxFreq);
    float applyScaleInv(float val, float minFreq, float maxFreq);

    
    class FilterBankObj
    {
    public:
        FilterBankObj();
        
        FilterBankObj(int dataSize, float sampleRate, int numFilters);
        
        virtual ~FilterBankObj();
        
    protected:
        friend class FilterBank;
        
        int _dataSize;
        float _sampleRate;
        int _numFilters;
        
        struct Filter
        {
            vector<float> _data;
            int _bounds[2];
        };
        
        vector<Filter> _filters;
    };
    
    void createFilterBankHzToTarget(FilterBankObj *filterBank, int dataSize,
                                    float sampleRate, int numFilters);
    void createFilterBankTargetToHz(FilterBankObj *filterBank, int dataSize,
                                    float sampleRate, int numFilters);
    void applyFilterBank(vector<float> *result,
                         const vector<float> &magns,
                         const FilterBankObj &filterBank);

    void fixSmallTriangles(float *fmin, float *fmax, int dataSize);
        
    
    FilterBankObj _hzToTargetFilterBank;
    FilterBankObj _targetToHzFilterBank;

    Scale::Type _targetScaleType;
    Scale *_scale;
    
private:
    // Tmp buffers
    vector<float> _tmpBuf0;
};

#endif
