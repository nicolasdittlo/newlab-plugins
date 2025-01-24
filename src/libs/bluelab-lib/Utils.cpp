/* Copyright (C) 2025 Nicolas Dittlo <bluelab.plugins@gmail.com>
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

#include <string.h>

#include "Defines.h"
#include "ParamSmoother.h"
#include "Utils.h"

void
Utils::complexToMagnPhase(vector<float> *resultMagns,
                          vector<float> *resultPhases,
                          const vector<complex<float> > &complexBuf)
{
    resultMagns->resize(complexBuf.size());
    resultPhases->resize(complexBuf.size());

    for (int i = 0; i < complexBuf.size(); i++)
    {
        const complex<float> &c = complexBuf[i];
        (*resultMagns)[i] = abs(c);
        (*resultPhases)[i] = arg(c);
    }
}

void
Utils::magnPhaseToComplex(vector<complex<float> > *complexBuf,
                          const vector<float> &magns,
                          const vector<float> &phases)
{
    complexBuf->resize(magns.size());

    for (int i = 0; i < magns.size(); i++)
        (*complexBuf)[i] = polar(magns[i], phases[i]);
}

void
Utils::complexToMagn(vector<float> *result, const vector<complex<float> > &complexBuf)
{
    result->resize(complexBuf.size());
    
    int complexBufSize = complexBuf.size();
    const complex<float> *complexBufData = complexBuf.data();
    float *resultData = result->data();
    
    for (int i = 0; i < complexBufSize; i++)
    {
        float magn = abs(complexBufData[i]);
        resultData[i] = magn;
    }
}

void
Utils::fillZero(vector<int> *buf)
{
    memset(buf->data(), 0, buf->size()*sizeof(int));
}

void
Utils::fillZero(vector<float> *buf)
{
    memset(buf->data(), 0, buf->size()*sizeof(float));
}

void
Utils::fillZero(vector<complex<float> > *buf)
{
    memset(buf->data(), 0, buf->size()*sizeof(complex<float>));
}

void
Utils::fillZero(vector<float> *ioBuf, int numZeros)
{
    if (numZeros > ioBuf->size())
        numZeros = ioBuf->size();
    
    memset(ioBuf->data(), 0, numZeros*sizeof(float));
}

void
Utils::fillValue(vector<float> *buf, float value)
{
    for (int i = 0; i < buf->size(); i++)
        (*buf)[i] = value;
}

void
Utils::resizeFillZeros(vector<float> *buf, int newSize)
{
    int prevSize = buf->size();
    buf->resize(newSize);
    
    if (newSize <= prevSize)
        return;
    
    memset(&buf->data()[prevSize], 0, (newSize - prevSize)*sizeof(float));
}

void
Utils::addBuffers(vector<float> *buf0, const vector<float> &buf1)
{    
    for (int i = 0; i < buf0->size(); i++)
        (*buf0)[i] += buf1[i];
}

void
Utils::addBuffers(vector<complex<float> > *buf0, const vector<complex<float> > &buf1)
{    
    for (int i = 0; i < buf0->size(); i++)
        (*buf0)[i] += buf1[i];
}

void
Utils::addBuffers(vector<float> *result, const vector<float> &buf0, const vector<float> &buf1)
{
    result->resize(buf0.size());
    
    for (int i = 0; i < buf0.size(); i++)
        (*result)[i] = buf0[i] + buf1[i];
}

void
Utils::multBuffers(vector<complex<float> > *buf0, const vector<float> &buf1)
{
    for (int i = 0; i < buf0->size(); i++)
        (*buf0)[i] *= buf1[i];
}

void
Utils::multBuffers(vector<complex<float> > *buf0, const vector<complex<float> > &buf1)
{
    for (int i = 0; i < buf0->size(); i++)
        (*buf0)[i] *= buf1[i];
}

void
Utils::multBuffers(vector<float> *buf,
                   const vector<float> &values)
{
    int bufSize = buf->size();
    float  *bufData = buf->data();
    const float *valuesData = values.data();
     
    for (int i = 0; i < bufSize; i++)
    {
        float val = valuesData[i];
        bufData[i] *= val;
    }
}

void
Utils::substractBuffers(vector<float> *ioBuf, const vector<float> &subBuf)
{
    int ioBufSize = ioBuf->size();
    float *ioBufData = ioBuf->data();
    const float *subBufData = subBuf.data();
        
    for (int i = 0; i < ioBufSize; i++)
    {
        float val = ioBufData[i];
        float sub = subBufData[i];
        
        val -= sub;
        
        ioBufData[i] = val;
    }
}

void
Utils::substractBuffers(vector<complex<float> > *buf0, const vector<complex<float> > &buf1)
{
    for (int i = 0; i < buf0->size(); i++)
        (*buf0)[i] -= buf1[i];
}

void
Utils::multValue(vector<float> *buf, float val)
{
    for (int i = 0; i < buf->size(); i++)
        (*buf)[i] *= val;
}

void
Utils::multValue(vector<complex<float> > *buf, float val)
{
    for (int i = 0; i < buf->size(); i++)
        (*buf)[i] *= val;
}

void
Utils::computeNormOpposite(vector<float> *buf)
{
    for (int i = 0; i < buf->size(); i++)
        (*buf)[i] = 1.0 - (*buf)[i];
}

void
Utils::computeSquareConjugate(vector<complex<float> > *buf)
{
    complex<float> conj;
    complex<float> tmp;
    for (int i = 0; i < buf->size(); i++)
    {
        complex<float> &c = (*buf)[i];
        complex<float> cj = std::conj(c);
        c = cj*c;
    }
}

float
Utils::computeSum(const vector<float> &buf)
{
    float result = 0.0f;
    
    for (int i = 0; i < buf.size(); i++)
        result += buf[i];
    
    return result;
}

void
Utils::insertValues(vector<float> *buf, int index, int numValues, float value)
{
    for (int i = 0; i < numValues; i++)
        buf->insert(buf->begin() + index, value);
}

void
Utils::removeValuesCyclic(vector<float> *buf, int index, int numValues)
{
    // Remove too many => empty the result
    if (numValues >= buf->size())
    {
        buf->resize(0);
        
        return;
    }
    
    // Manage negative index
    if (index < 0)
        index += buf->size();
    
    // Prepare the result with the new size
    vector<float> result;
    result.resize(buf->size() - numValues);
    Utils::fillZero(&result);
    
    // Copy cyclicly
    int bufPos = index + 1;
    int resultPos = index + 1 - numValues;
    while (resultPos < 0)
        resultPos += result.size();
    
    int resultSize = result.size();
    float *resultData = result.data();
    int bufSize = buf->size();
    float *bufData = buf->data();
    
    for (int i = 0; i < resultSize; i++)
    {
        bufPos = bufPos % bufSize;
        resultPos = resultPos % resultSize;
        
        float val = bufData[bufPos];
        resultData[resultPos] = val;
        
        bufPos++;
        resultPos++;
    }
    
    *buf = result;
}

float
Utils::ampToDB(float sampleVal)
{
#define AMP_DB 8.685889638065036553
    
    float result = AMP_DB*log(fabs(sampleVal));

    return result;
}

float
Utils::ampToDB(float sampleVal, float eps, float minDB)
{
#define AMP_DB 8.685889638065036553
    
    float result = minDB;
    float absSample = fabs(sampleVal);
    if (absSample > eps)
        result = AMP_DB*log(fabs(sampleVal));

    return result;
}

float
Utils::DBToAmp(float dbVal)
{
    // Magic number for dB to gain conversion.
    // Approximates 10^(x/20)
#define IAMP_DB 0.11512925464970
 
    return exp(((float)IAMP_DB)*dbVal);
}

void
Utils::DBToAmp(vector<float> *ioBuf)
{
    int ioBufSize = ioBuf->size();
    float *ioBufData = ioBuf->data();
    
    for (int i = 0; i < ioBufSize; i++)
    {
        float db = ioBufData[i];
        
        float amp = Utils::DBToAmp(db);
        
        ioBufData[i] = amp;
    }
}

void
Utils::ampToDB(vector<float> *dBBuf, const vector<float> &ampBuf, float eps, float minDB)
{
    dBBuf->resize(ampBuf.size());
    
    for (int i = 0; i < ampBuf.size(); i++)
    {
        float amp = ampBuf[i];
        float dbAmp = ampToDB(ampBuf[i], eps, minDB);
        (*dBBuf)[i] = dbAmp;
    }
}

void
Utils::ampToDB(float *dBBuf, const float *ampBuf, int bufSize,
               float eps, float minDB)
{
    for (int i = 0; i < bufSize; i++)
    {
        float amp = ampBuf[i];
        float dbAmp = ampToDB(amp, eps, minDB);
        
        dBBuf[i] = dbAmp;
    }
}

float
Utils::normalizedYTodB(float y, float mindB, float maxdB)
{
    if (fabs(y) < BL_EPS)
        y = mindB;
    else
        y = Utils::ampToDB(y);
    
    y = (y - mindB)/(maxdB - mindB);
    
    return y;
}

void
Utils::normalizedYTodB(const vector<float> &yBuf,
                       float mindB, float maxdB,
                       vector<float> *resBuf)
{
    resBuf->resize(yBuf.size());

    float rangeInv = 1.0/(maxdB - mindB);

    int bufSize = yBuf.size();
    const float *yBufData = yBuf.data();
    float *resBufData = resBuf->data();
    
    for (int i = 0; i < bufSize; i++)
    {
        float y = yBufData[i];
        
        if (fabs(y) < BL_EPS)
            y = mindB;
        else
            y = Utils::ampToDB(y);
    
        y = (y - mindB)*rangeInv;

        resBufData[i] = y;
    }
}

float
Utils::normalizedYTodBInv(float y, float mindB, float maxdB)
{
    float result = y*(maxdB - mindB) + mindB;
    
    result = Utils::DBToAmp(result);
    
    return result;
}

float
Utils::applyGamma(float t, float gamma)
{
    float bA = t/((1.0/gamma - 2.0)*(1.0 - t) + 1.0);
    return bA;
}

void
Utils::clipMax(vector<float> *values, float maxValue)
{
    int valuesSize = values->size();
    float *valuesBuf = values->data();
        
    for (int i = 0; i < valuesSize; i++)
    {
        float val = valuesBuf[i];
        
        if (val > maxValue)
            val = maxValue;
        
        valuesBuf[i] = val;
    }
}

void
Utils::clipMin(vector<float> *values, float minVal)
{
    int valuesSize = values->size();
    float *valuesData = values->data();
       
    for (int i = 0; i < valuesSize; i++)
    {
        float val = valuesData[i];
        if (val < minVal)
            val = minVal;
        
        valuesData[i] = val;
    }
}

void
Utils::FftIdsToSamplesIds(const vector<float> &phases, vector<int> *samplesIds)
{
    samplesIds->resize(phases.size());
    Utils::fillZero(samplesIds);
    
    int bufSize = phases.size();
    const float *phasesData = phases.data();
    int *samplesIdsData = samplesIds->data();
    
    float prev = 0.0;
    for (int i = 0; i < bufSize; i++)
    {
        float phase = phasesData[i];
        
        float phaseDiff = phase - prev;
        prev = phase;
        
        // Avoid having a big phase diff due to prev == 0
        if (i == 0)
            continue;
        
        // TODO: optimize this !
        while(phaseDiff < 0.0)
            phaseDiff += 2.0*M_PI;
        
        float samplePos = ((float)bufSize)*phaseDiff/(2.0*M_PI);
        
        samplesIdsData[i] = (int)samplePos;
    }
}

void
Utils::reverse(vector<float> *values)
{    
    int valuesHalfSize = values->size()/2;
    for (int i = 0; i < valuesHalfSize; i++)
    {
        float val0 = values->data()[i];
        int idx = values->size() - i - 1;
        float val1 = values->data()[idx];

        values->data()[i] = val1;
        values->data()[idx] = val0;
    }
}

void
Utils::append(vector<float> *vec, const float *buf, int size)
{
    int prevSize = vec->size();
    vec->resize(prevSize + size);

    memcpy(&vec->data()[prevSize], buf, size*sizeof(float));
}

void
Utils::copyBuf(float *toBuf, const float *fromData, int fromSize)
{
    memcpy(toBuf, fromData, fromSize*sizeof(float));
}

void
Utils::fillSecondFftHalf(const vector<complex<float> > &inHalfBuf,
                         vector<complex<float> > *outBuf)
{
    if (inHalfBuf.size() < 1)
        return;
    outBuf->resize(inHalfBuf.size()*2);

    memcpy(outBuf->data(), inHalfBuf.data(), inHalfBuf.size()*sizeof(complex<float>));
    
    int ioBufferSize2 = inHalfBuf.size();
    complex<float> *ioBufferData = outBuf->data();
    
    for (int i = 0; i < ioBufferSize2; i++)
    {
        int id0 = i + ioBufferSize2;
        int id1 = ioBufferSize2 - i;
        
        ioBufferData[id0] = conj(ioBufferData[id1]);
    }
}

int
Utils::nearestPowerOfTwo(int value)
{
    // Calculate lower and upper powers of two
    int lowerPower = pow(2, floor(log2(value)));
    int upperPower = pow(2, ceil(log2(value)));

    // Return the closer power of two
    if (abs(value - lowerPower) < abs(value - upperPower))
        return lowerPower;
    else
        return upperPower;
}

void
Utils::smooth(vector<float> *ioCurrentValues,
              vector<float> *ioPrevValues,
              float smoothFactor)
{
    if (ioCurrentValues->size() != ioPrevValues->size())
    {
        *ioPrevValues = *ioCurrentValues;
        
        return;
    }
    
    for (int i = 0; i < ioCurrentValues->size(); i++)
    {
        float val = ioCurrentValues->data()[i];
        float prevVal = ioPrevValues->data()[i];
        
        float newVal = smoothFactor*prevVal + (1.0 - smoothFactor)*val;
        
        ioCurrentValues->data()[i] = newVal;
    }
    
    *ioPrevValues = *ioCurrentValues;
}

void
Utils::unwrapPhases(vector<float> *phases, bool adjustFirstPhase)
{
    if (phases->size() == 0)
        // Empty phases
        return;
    
    float prevPhase = phases->data()[0];

    if (adjustFirstPhase)
        findNextPhase(&prevPhase, (float)0.0);
    
    int phasesSize = phases->size();
    float *phasesData = phases->data();
    for (int i = 0; i < phasesSize; i++)
    {
        float phase = phasesData[i];
        
        findNextPhase(&phase, prevPhase);
        
        phasesData[i] = phase;
        
        prevPhase = phase;
    }
}

void
Utils::findNextPhase(float *phase, float refPhase)
{
    if (*phase >= refPhase)
        return;
    
    float refMod = fmod_negative(refPhase, (float)TWO_PI);
    float pMod = fmod_negative(*phase, (float)TWO_PI);
    
    float resPhase = (refPhase - refMod) + pMod;
    if (resPhase < refPhase)
        resPhase += TWO_PI;
    
    *phase = resPhase;
}

float
Utils::fmod_negative(float x, float y)
{
    // Move input to range 0.. 2*pi
    if (x < 0.0)
    {
        // fmod only supports positive numbers. Thus we have
        // to emulate negative numbers
        float modulus = x * -1.0;
        modulus = fmod(modulus, y);
        modulus = -modulus + y;
        
        return modulus;
    }
    return fmod(x, y);
}

float
Utils::princarg(float x)
{
    float result = Utils::fmod_negative(x + M_PI, TWO_PI) - M_PI;

    return result;
}

static bool
ccw(float A[2], float B[2], float C[2])
{
    bool res = (C[1] - A[1])*(B[0] - A[0]) > (B[1] - A[1])*(C[0] - A[0]);

    return res;
}

bool
Utils::segSegIntersect(float seg0[2][2], float seg1[2][2])
{
    bool res = ((ccw(seg0[0], seg1[0], seg1[1]) != ccw(seg0[1], seg1[0], seg1[1])) &&
                (ccw(seg0[0], seg0[1], seg1[0]) != ccw(seg0[0], seg0[1], seg1[1])));
    
    return res;
}

float
Utils::trapezoidArea(float a, float b, float h)
{
    float area = (a + b)*h*0.5;
    
    return area;
}

float
Utils::computeMin(const vector<float> &buf)
{
    int bufSize = buf.size();
    const float *bufData = buf.data();
    
    if (bufSize == 0)
        return 0.0;
    
    float minVal = bufData[0];
    
    for (int i = 0; i < bufSize; i++)
    {
        float val = bufData[i];
        
        if (val < minVal)
            minVal = val;
    }
    
    return minVal;
}

float
Utils::computeMax(const vector<float> &buf)
{
    int bufSize = buf.size();
    const float *bufData = buf.data();
    
    if (bufSize == 0)
        return 0.0;
    
    float maxVal = bufData[0];
    
    for (int i = 0; i < bufSize; i++)
    {
        float val = bufData[i];
        
        if (val > maxVal)
            maxVal = val;
    }
    
    return maxVal;
}

float
Utils::normalize(float value, float minimum, float maximum)
{
    if (fabs(maximum - minimum) > 0.0)
        value = (value - minimum)/(maximum - minimum);
    else
        value = 0.0;

    return value;
}

void
Utils::normalize(vector<float> *values,
                 float minimum, float maximum)
{
    for (int i = 0; i < values->size(); i++)
    {
        float val = values->data()[i];
        
        if (fabs(maximum - minimum) > 0.0)
            val = (val - minimum)/(maximum - minimum);
        else
            val = 0.0;
        
        values->data()[i] = val;
    }
}

void
Utils::normalize(vector<float> *values)
{
    float minimum = Utils::computeMin(*values);
    float maximum = Utils::computeMax(*values);
    
    for (int i = 0; i < values->size(); i++)
    {
        float val = values->data()[i];
        
        if (fabs(maximum - minimum) > 0.0)
            val = (val - minimum)/(maximum - minimum);
        else
            val = 0.0;
        
        values->data()[i] = val;
    }
}

void
Utils::mixParamToCoeffs(float mix,
                        float *coeff0, float *coeff1,
                        float coeff1Scale)
{    
    if (mix <= 0.0)
    {
        *coeff0 = 1.0;
        *coeff1 = 1.0 + mix;
    }
    else if (mix > 0.0)
    {
        *coeff0 = 1.0 - mix;
        *coeff1 = 1.0;
    }

    if (mix > 0.0)
        *coeff1 = mix*(coeff1Scale - 1) + 1.0;
}

void
Utils::computeOpposite(vector<float> *buf)
{
    for (int i = 0; i < buf->size(); i++)
    {
        float val = buf->data()[i];
        val = 1.0f - val;
        buf->data()[i] = val;
    }
}

void
Utils::applyGain(const vector<float> &in, vector<float> *out, ParamSmoother *smoother)
{    
    for (int i = 0; i < in.size(); i++)
    {
        float gain = smoother->process();

        (*out)[i] = in[i]*gain;
    }
}

void
Utils::fillMissingValues(vector<float> *values,
                         bool extendBounds, float undefinedValue)
{
    if (extendBounds)
        // Extend the last value to the end
    {
        // Find the last max
        int lastIndex = values->size() - 1;
        float lastValue = undefinedValue;
        
        int valuesSize = values->size();
        float *valuesData = values->data();
        
        for (int i = valuesSize - 1; i > 0; i--)
        {
            float val = valuesData[i];
            if (val > undefinedValue)
            {
                lastValue = val;
                lastIndex = i;
                
                break;
            }
        }
        
        // Fill the last values with last max
        for (int i = valuesSize - 1; i > lastIndex; i--)
            valuesData[i] = lastValue;
    }
    
    // Fill the holes by linear interpolation
    float startVal = undefinedValue;
    
    // First, find start val
    
    int valuesSize = values->size();
    float *valuesData = values->data();
    
    for (int i = 0; i < valuesSize; i++)
    {
        float val = valuesData[i];
        if (val > undefinedValue)
            startVal = val;
    }
    
    int loopIdx = 0;
    int startIndex = 0;
    
    // Then start the main loop
    while(loopIdx < valuesSize)
    {
        float val = valuesData[loopIdx];
        
        if (val > undefinedValue)
            // Defined
        {
            startVal = val;
            startIndex = loopIdx;
            
            loopIdx++;
        }
        else
            // Undefined
        {
            // Start at 0
            if (!extendBounds &&
                (loopIdx == 0))
                startVal = undefinedValue;
            
            // Find how many missing values we have
            int endIndex = startIndex + 1;
            float endVal = undefinedValue;
            bool defined = false;
            
            while(endIndex < valuesSize)
            {
                if (endIndex < valuesSize)
                    endVal = valuesData[endIndex];
                
                defined = (endVal > undefinedValue);
                if (defined)
                    break;
                
                endIndex++;
            }
            
            // Fill the missing values with lerp
            for (int i = startIndex; i < endIndex; i++)
            {
                float t =
                    ((float)(i - startIndex))/(endIndex - startIndex);
                
                float newVal = (1.0 - t)*startVal + t*endVal;
                
                valuesData[i] = newVal;
            }
            
            startIndex = endIndex;
            loopIdx = endIndex;
        }
    }
}

int
Utils::findMaxIndex(const vector<float> &values,
                    int startIdx, int endIdx)
{
    int maxIndex = -1;
    float maxValue = -BL_INF;
    
    int valuesSize = values.size();
    const float *valuesData = values.data();

    if (endIdx > valuesSize - 1)
        endIdx = valuesSize - 1;
    
    for (int i = startIdx; i <= endIdx; i++)
    {
        float value = valuesData[i];
        
        if (value > maxValue)
        {
            maxValue = value;
            maxIndex = i;
        }
    }
    
    return maxIndex;
}
