/* Copyright (C) 2025 Nicolas Dittlo <newlab.plugins@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this software; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <complex>

using namespace std;

class ParamSmoother;
class Utils
{
 public:
    static void complexToMagnPhase(vector<float> *resultMagns,
                                   vector<float> *resultPhases,
                                   const vector<complex<float> > &complexBuf);

    static void magnPhaseToComplex(vector<complex<float> > *complexBuf,
                                   const vector<float> &magns,
                                   const vector<float> &phases);

    static void complexToMagn(vector<float> *result, const vector<complex<float> > &complexBuf);
        
    static void fillZero(vector<int> *buf);
    static void fillZero(vector<float> *buf);
    static void fillZero(vector<complex<float> > *buf);
    static void fillZero(vector<float> *ioBuf, int numZeros);
        
    static void fillValue(vector<float> *buf, float value);
    
    static void resizeFillZeros(vector<float> *buf, int newSize);

    static void addBuffers(vector<float> *buf0, const vector<float> &buf1);
    static void addBuffers(vector<complex<float> > *buf0, const vector<complex<float> > &buf1);
    static void addBuffers(vector<float> *result, const vector<float> &buf0, const vector<float> &buf1);
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

    static void fillSecondFftHalf(const vector<complex<float> > &inHalfBuf,
                                  vector<complex<float> > *outBuf);
        
    static void FftIdsToSamplesIds(const vector<float> &phases, vector<int> *samplesIds);

    static void reverse(vector<float> *values);

    static void append(vector<float> *vec, const float *buf, int size);

    static void copyBuf(float *toBuf, const float *fromData, int fromSize);

    static int nearestPowerOfTwo(int value);

    static void smooth(vector<float> *ioCurrentValues,
                       vector<float> *ioPrevValues,
                       float smoothFactor);

    static void unwrapPhases(vector<float> *phases, bool adjustFirstPhase = true);

    static void findNextPhase(float *phase, float refPhase);
        
    static float fmod_negative(float x, float y);

    static float princarg(float x);
        
    static bool segSegIntersect(float seg0[2][2], float seg1[2][2]);

    static float trapezoidArea(float a, float b, float h);

    static float computeMin(const vector<float> &buf);

    static float computeMax(const vector<float> &buf);

    static float normalize(float value, float minimum, float maximum);
    
    static void normalize(vector<float> *values, float minimum, float maximum);

    static void normalize(vector<float> *values);

    static void mixParamToCoeffs(float mix,
                                 float *coeff0, float *coeff1,
                                 float coeff1Scale = 1.0f);

    static void computeOpposite(vector<float> *buf);

    static void applyGain(const vector<float> &in, vector<float> *out, ParamSmoother *smoother);
};

#endif
