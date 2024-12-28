#ifndef TRANSIENT_SHAPER_PROCESSOR_H
#define TRANSIENT_SHAPER_PROCESSOR_H

#include "OverlapAdd.h"

class TransientShapeProcessor : public OverlapAddProcessor
{
public:
    TransientShapeProcessor(float sampleRate);
    
    virtual ~TransientShapeProcessor();

    void reset(float sampleRate);
    
    void setPrecision(float precision);
    
    void setSoftHard(float softHard);
    
    void setFreqAmpRatio(float ratio);
    
    void processFFT(vector<complex<float> > *ioBuffer) override;
    
    void getTransientness(vector<float> *outTransientness);
    
    void applyTransientness(vector<float> *ioSamples,
                            const vector<float> &currentTransientness);

protected:
    float computeMaxTransientness();

    float _softHard;
    float _precision;
    
    float _freqAmpRatio;
    
    vector<float> _transientness;
    
    // For computing derivative (for amp to trans)
    vector<float> _prevPhases;
        
    TransientLib *_transLib;
    
private:
    // Tmp buffers
    vector<float> _tmpBuf0;
    vector<complex<float> > _tmpBuf1;
    vector<float> _tmpBuf2;
    vector<float> _tmpBuf3;
    vector<float> _tmpBuf4;
    vector<float> _tmpBuf5;
    vector<float> _tmpBuf6;
    vector<float> _tmpBuf7;
    vector<float> _tmpBuf8;
    vector<float> _tmpBuf9;
    vector<float> _tmpBuf10;
    vector<float> _tmpBuf11;
    vector<float> _tmpBuf12;
    vector<float> _tmpBuf13;
    vector<float> _tmpBuf14;
};

#endif
