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

#include <string.h>

#include "Defines.h"

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
    if (resultPos < 0)
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
    if (fabs(y) < NL_EPS)
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
        
        if (fabs(y) < NL_EPS)
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
