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
