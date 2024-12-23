#ifndef UTILS_H
#define UTILS_H

#include <vector>
using namespace std;

class Utils
{
 public:
    static void complexToMagnPhase(vector<float> *resultMagn,
                                   vector<float> *resultPhase,
                                   const vector<complex> &complexBuf);

    static void magnPhaseToComplex(vector<complex> *complexBuf,
                                   const vector<float> &magns,
                                   const vector<float> &phases);
        
    static void fillZero(vector<float> *buff);

    static void resizeFillZeros(vector<float> *buf, int newSize);

    static void addBuffers(vector<float> *buf0, const vector<float> &buf1);
    static void multBuffers(vector<complex> *buf0, const vector<float> &buf1);
    static void substractBuffers(vector<complex> *buf0, const vector<complex> &buf1);

    static void multValue(vector<float> *buf, float val);
        
    static void computeNormOpposite(vector<float> *buf);

    static void computeSquareConjugate(vector<complex> *buf);

    static float computeSum(const vector<float> &buf);
        
    static float ampToDB(float sampleVal, float eps, float minDB);
        
    static void ampToDB(vector<float> *dBBuf, const vector<float> &ampBuf, float eps, float minDB);
};

#endif
