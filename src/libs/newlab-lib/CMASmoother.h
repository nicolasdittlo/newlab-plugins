#ifndef CMA_SMOOTHER_H
#define CMA_SMOOTHER_H

// Central moving average smoother
class CMASmoother
{
public:
    CMASmoother(int bufferSize, int windowSize);
    
    virtual ~CMASmoother();

    void reset();

    void reset(int bufferSize, int windowSize);
    
    // Return true if nFrames has been returned
    bool process(const float *data, float *smoothedData, int nFrames);

    bool processOne(const float *data, float *smoothedData,
                    int nFrames, int windowSize);
    
protected:
    bool processInternal(const float *data, float *smoothedData, int nFrames);
    
    // Return true if something has been processed
    bool centralMovingAverage(vector<float> &inData,
                              vector<float> &outData, int windowSize);
    
    void manageConstantValues(const float *data, int nFrames);

    int _bufferSize;
    int _windowSize;
    
    bool _firstTime;
    
    float _prevVal;
    
    vector<float> _inData;
    vector<float> _outData;

private:
    // Tmp buffers
    vector<float> _tmpBuf0;
    vector<float> _tmpBuf1;
};

#endif
