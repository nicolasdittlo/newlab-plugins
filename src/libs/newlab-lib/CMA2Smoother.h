#ifndef CMA2_SMOOTHER_H
#define CMA2_SMOOTHER_H

#include <vector>
using namespace std;

#include "CMASmoother.h"

// Double central moving average smoother
class CMA2Smoother
{
public:
    CMA2Smoother(int bufferSize, int windowSize);
    
    virtual ~CMA2Smoother();
    
    // Return true if nFrames has been returned
    bool process(const float *data, float *smoothedData, int nFrames);

    bool processOne(const float *data,
                    float *smoothedData,
                    int nFrames, int windowSize);
    
    bool processOne(const vector<float> &inData,
                    vector<float> *outSmoothedData,
                    int windowSize);
    
    void reset();
    
protected:
    CMASmoother _smoother0;
    CMASmoother _smoother1;

    // For processOne()
    CMASmoother _smootherP1;

private:
    // Tmp buffers
    vector<float> _tmpBuf0;
};

#endif
