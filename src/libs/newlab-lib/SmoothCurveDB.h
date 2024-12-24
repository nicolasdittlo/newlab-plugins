#ifndef SMOOTH_CURVE_DB_H
#define SMOOTH_CURVE_DB_H

#include <vector>
using namespace std;

class Curve;
class SmoothCurveDB
{
    SmoothCurveDB(Curve *curve,
                  float smoothFactor,
                  int size, float defaultValue,
                  float minDB, float maxDB,
                  float sampleRate);
    
    virtual ~SmoothCurveDB();

    void reset(float sampleRate, float smoothFactor);

    void clearValues();
    
    void setValues(const vector<float> &values, bool reset);
};

#endif
