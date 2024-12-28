#include "CMA2Smoother.h"

CMA2Smoother::CMA2Smoother(int bufferSize, int windowSize)
: _smoother0(bufferSize, windowSize),
  _smoother1(bufferSize, windowSize),
  _smootherP1(bufferSize, windowSize) {}

CMA2Smoother::~CMA2Smoother() {}


bool
CMA2Smoother::process(const float *data, float *smoothedData, int nFrames)
{
    if (data == NULL)
        return false;
    
    vector<float> tmpData;
    tmpData.resize(nFrames);
    
    bool processed = _smoother0.process(data, tmpData.data(), nFrames);
    if (processed)
        processed = _smoother1.process(tmpData.data(), smoothedData, nFrames);
    
    return processed;
}

bool
CMA2Smoother::processOne(const float *data, float *smoothedData,
                         int nFrames, int windowSize)
{
    if (windowSize <= 1)
        return false;
    
    vector<float> &tmpData = _tmpBuf0;
    tmpData.resize(nFrames);
    
    bool processed = _smootherP1.processOne(data, tmpData.data(),
                                            nFrames, windowSize);
    if (processed)
        processed = _smootherP1.processOne(tmpData.data(), smoothedData,
                                           nFrames, windowSize);
    
    return processed;
}

bool
CMA2Smoother::processOne(const vector<float> &inData,
                         vector<float> *outSmoothedData,
                         int windowSize)
{
    outSmoothedData->resize(inData.size());
    
    bool res = processOne(inData.data(), outSmoothedData->data(),
                          inData.size(), windowSize);
    
    return res;
}

void
CMA2Smoother::reset()
{
    _smoother0.reset();
    _smoother1.reset();
}
