#ifndef SMOOTH_AVG_HISTOGRAM_DB_H
#define SMOOTH_AVG_HISTOGRAM_DB_H

#include <vector>
using namespace std;

// Normalize Y to dB internally
class SmoothAvgHistogramDB
{
public:
    SmoothAvgHistogramDB(int size, float smoothCoeff,
                         float defaultValue, float mindB, float maxdB);
    
    virtual ~SmoothAvgHistogramDB();
    
    void addValues(const vector<float> &values);
    
    int getNumValues();

    // Get values scaled back to amp
    void getValues(vector<float> *values);

    // Get internal values which are in DB
    void getValuesDB(vector<float> *values);
    
    // Force the internal values to be the new values,
    // (withtout smoothing)
    void setValues(const vector<float> *values,
                   bool convertToDB = false);
    
    void reset(float smoothCoeff = -1.0);
    
protected:
    vector<float> _data;

    // If smooth is 0, then we get instantaneous value.
    float _smoothCoeff;
    
    float _defaultValue;
    
    float _mindB;
    float _maxdB;

 private:
    // Tmp buffers
    vector<float> _tmpBuf0;
};

#endif
