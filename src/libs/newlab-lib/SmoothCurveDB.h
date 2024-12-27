#ifndef SMOOTH_CURVE_DB_H
#define SMOOTH_CURVE_DB_H

#include <vector>
using namespace std;

class Curve;
class SmoothAvgHistogramDB;
class SmoothCurveDB
{
 public:
    SmoothCurveDB(Curve *curve,
                  float smoothFactor,
                  int size, float defaultValue,
                  float minDB, float maxDB,
                  float sampleRate);
    
    virtual ~SmoothCurveDB();

    void reset(float sampleRate, float smoothFactor);

    void clearValues();
    
    void setValues(const vector<float> &values);

 protected:
    float _minDB;
    float _maxDB;
    
    SmoothAvgHistogramDB *_histogram;
    
    Curve *_curve;

    float _sampleRate;

 private:
    vector<float> _tmpBuf0;
    vector<float> _tmpBuf1;
    vector<float> _tmpBuf2;
};

#endif
