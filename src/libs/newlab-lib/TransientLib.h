#ifndef TRANSIENT_LIB_H
#define TRANSIENT_LIB_H

#include <vector>
using namespace std;

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

    CMA2Smoother *_smoother;

    vector<float> _tmpBuf0;
    vector<float> _tmpBuf1;
    vector<int> _tmpBuf2;
    vector<float> _tmpBuf3;
};

#endif
