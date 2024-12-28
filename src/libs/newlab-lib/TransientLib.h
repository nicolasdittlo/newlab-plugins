#ifndef TRANSIENT_LIB_H
#define TRANSIENT_LIB_H

class CMA2Smoother;
class TransientLib
{
 public:
    TransientLib();
    
    virtual ~TransientLib();

    void computeTransientness(const vector<float> &magns,
                              const vector<float> &phases,
                              const vector<float> *prevPhases,
                              float freqAmpRatio,
                              float smoothFactor,
                              float sampleRate,
                              vector<float> *transientness);
 protected:
    void smoothTransients(vector<float> *transients, float smoothFactor);
};

#endif
