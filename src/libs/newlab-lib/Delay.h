#ifndef DELAY_H
#define DELAY_H

#include <vector>
using namespace std;

class Delay
{
public:
    Delay(float delay);
    
    Delay(const Delay &other);
    
    virtual ~Delay();
    
    void reset();
    
    void setDelay(float delay);
    
    float processSample(float sample);

    void processSamples(vector<float> *samples);
    
protected:
    float _delay;
    
    vector<float> _delayLine;
    
    long _readPos;
    long _writePos;
};

#endif
