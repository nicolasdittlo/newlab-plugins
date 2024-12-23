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
        
    static void fillZero(vector<float> *buf);
    static void fillZero(vector<complex<float> > *buf);

    static void resizeFillZeros(vector<float> *buf, int newSize);

    static void addBuffers(vector<float> *buf0, const vector<float> &buf1);
    static void multBuffers(vector<complex<float>> *buf0, const vector<float> &buf1);
    static void multBuffers(vector<complex<float>> *buf0, const vector<complex<float> > &buf1);
    static void substractBuffers(vector<complex<float>> *buf0, const vector<complex<float> > &buf1);

    static void multValue(vector<float> *buf, float val);
    static void multValue(vector<complex<float> > *buf, float val);
        
    static void computeNormOpposite(vector<float> *buf);

    static void computeSquareConjugate(vector<complex<float> > *buf);

    static float computeSum(const vector<float> &buf);
        
    static float ampToDB(float sampleVal, float eps, float minDB);
        
    static void ampToDB(vector<float> *dBBuf, const vector<float> &ampBuf, float eps, float minDB);

    static void ampToDB(float *dBBuf, const float *ampBuf, int bufSize,
                        float eps, float minDB);
};

#endif
