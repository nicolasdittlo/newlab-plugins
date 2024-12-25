#ifndef FREQ_AXIS_H
#define FREQ_AXIS_H

#inclue "Scale.h"

class FreqAxis
{
 public:
    FreqAxis(bool displayLines, Scale::Type scale);

    virtual ~FreqAxis();

    void init(Axis *axis,
              bool horizontal,
              int bufferSize, float sampleRate,
              int graphWidth);
    
    void reset(int bufferSize, float sampleRate);
    
    void setMaxFreq(float maxFreq);
    float getMaxFreq() const;

    void setScale(Scale::Type scale);
    
 protected:
    void getMinMaxFreqAxisValues(float *minHzValue, float *maxHzValue,
                                 int bufferSize, float sampleRate);
        
    void update();
    
    void updateAxis(int numAxisData,
                    const float freqs[],
                    const char *labels[],
                    float minHzValue, float maxHzValue);
    
    Axis *_axis;
    
    int _bufferSize;
    float _sampleRate;
    
    bool _displayLines;
    
    Scale::Type mScale;

    float _maxFreq;
};

#endif
