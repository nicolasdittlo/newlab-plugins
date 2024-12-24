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
};

#endif
