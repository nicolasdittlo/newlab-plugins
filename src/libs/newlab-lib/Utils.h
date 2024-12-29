#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <complex>

using namespace std;

class Utils
{
 public:
    static void complexToMagnPhase(vector<float> *resultMagns,
                                   vector<float> *resultPhases,
                                   const vector<complex<float> > &complexBuf);

    static void magnPhaseToComplex(vector<complex<float> > *complexBuf,
                                   const vector<float> &magns,
                                   const vector<float> &phases);

    static void fillZero(vector<int> *buf);
    static void fillZero(vector<float> *buf);
    static void fillZero(vector<complex<float> > *buf);
    static void fillZero(vector<float> *ioBuf, int numZeros);
        
    static void fillValue(vector<float> *buf, float value);
    
    static void resizeFillZeros(vector<float> *buf, int newSize);

    static void addBuffers(vector<float> *buf0, const vector<float> &buf1);
    static void multBuffers(vector<float> *buf, const vector<float> &values);
    static void multBuffers(vector<complex<float>> *buf0, const vector<float> &buf1);
    static void multBuffers(vector<complex<float>> *buf0, const vector<complex<float> > &buf1);
    static void substractBuffers(vector<complex<float>> *buf0, const vector<complex<float> > &buf1);

    static void multValue(vector<float> *buf, float val);
    static void multValue(vector<complex<float> > *buf, float val);
        
    static void computeNormOpposite(vector<float> *buf);

    static void computeSquareConjugate(vector<complex<float> > *buf);

    static float computeSum(const vector<float> &buf);

    static void insertValues(vector<float> *buf, int index, int numValues, float value);
    static void removeValuesCyclic(vector<float> *buf, int index, int numValues);
        
    static float ampToDB(float sampleVal);
    static float ampToDB(float sampleVal, float eps, float minDB);

    static float DBToAmp(float dbVal);
    static void DBToAmp(vector<float> *ioBuf);
        
    static void ampToDB(vector<float> *dBBuf, const vector<float> &ampBuf, float eps, float minDB);

    static void ampToDB(float *dBBuf, const float *ampBuf, int bufSize,
                        float eps, float minDB);

    static float normalizedYTodB(float y, float mindB, float maxdB);

    static void normalizedYTodB(const vector<float> &yBuf,
                                float mindB, float maxdB,
                                vector<float> *resBuf);

    static float normalizedYTodBInv(float y, float mindB, float maxdB);
        
    static float applyGamma(float t, float gamma);

    static void clipMax(vector<float> *values, float maxValue);

    static void clipMin(vector<float> *values, float minVal);
        
    static void FftIdsToSamplesIds(const vector<float> &phases, vector<int> *samplesIds);

    static void reverse(vector<float> *values);

    static void append(vector<float> *vec, const float *buf, int size);

    static void copyBuf(float *toBuf, const float *fromData, int fromSize);
};

#endif
