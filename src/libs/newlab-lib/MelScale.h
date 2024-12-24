#ifndef MEL_SCALE_H
#define MEL_SCALE_H

#include <vector>
using namespace std;

class MelScale
{
public:
    MelScale();
    virtual ~MelScale();
    
    static float hzToMel(float freq);
    static float melToHz(float mel);
    
    // Quick transformations, without filtering
    static void hzToMel(vector<float> *resultMagns,
                        const vector<float> &magns,
                        float sampleRate);
    static void melToHz(vector<float> *resultMagns,
                        const vector<float> &magns,
                        float sampleRate);
    
    // Use real Hz to Mel conversion, using filters
    void hzToMelFilter(vector<float> *result,
                       const vector<float> &magns,
                       float sampleRate, int numFilters);
    // Inverse
    void melToHzFilter(vector<float> *result,
                       const vector<float> &magns,
                       float sampleRate, int numFilters);
    
protected:
    float computeTriangleAreaBetween(float txmin,
                                     float txmid,
                                     float txmax,
                                     float x0, float x1);
    static float computeTriangleY(float txmin, float txmid, float txmax,
                                  float x);
    
    class FilterBank
    {
    public:
        FilterBank();
        
        FilterBank(int dataSize, float sampleRate, int numFilters);
        
        virtual ~FilterBank();
        
    protected:
        friend class MelScale;
        
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
    
    void createFilterBankHzToMel(FilterBank *filterBank, int dataSize,
                                 float sampleRate, int numFilters);
    void createFilterBankMelToHz(FilterBank *filterBank, int dataSize,
                                 float sampleRate, int numFilters);
    void applyFilterBank(vector<float> *result,
                         const vector<float> &magns,
                         const FilterBank &filterBank);
    
    
    FilterBank _hzToMelFilterBank;
    FilterBank _melToHzFilterBank;

 private:
    vector<float> _tmpBuf0;
};

#endif
