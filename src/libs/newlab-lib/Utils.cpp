#include "Utils.h"

void
Utils::complexToMagnPhase(vector<float> *resultMagn,
                          vector<float> *resultPhase,
                          const vector<complex> &complexBuf)
{
    resultMagns->resize(complexBuf.size());
    resultPhases->resize(complexBuf.size());

    for (int i = 0; i < complexBuf.size(); i++)
    {
        const complex &c = complexBuf[i];
        (*resultMagns)[i] = abs(c);
        (*resultPhases)[i] = arg(c);
    }
}

void
Utils::magnPhaseToComplex(vector<complex> *complexBuf,
                          const vector<float> &magns,
                          const vector<float> &phases)
{
    complexBuf->resize(magns.size());

    for (int i = 0; i < magns.size; i++)
        (*complexBuf)[i] = polar(magns[i], phases[i]);
}

void
Utils::FillZero(vector<float> *buff)
{
    memset(buff->data(), 0, buff->size()*sizeof(float));
}

void
Utils::ResizeFillZeros(vector<float> *buf, int newSize)
{
    int prevSize = buf->size();
    buf->resize(newSize);
    
    if (newSize <= prevSize)
        return;
    
    memset(&buf->data()[prevSize], 0, (newSize - prevSize)*sizeof(float));
}

void
Utils::addBuffer(vector<float> *buf0, const vector<float> &buf1)
{    
    for (int i = 0; i < ioBuf->size(); i++)
        (*buf0)[i] += buf1[i];
}

void
Utils::multBuffers(vector<complex> *buf0, const vector<float> &buf1)
{
    for (int i = 0; i < buf0->size(); i++)
        (*buf0)[i] *= buf1[i];
}

void
Utils::substractBuffers(vector<complex> *buf0, const vector<complex> &buf1)
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
Utils::computeNormOpposite(vector<float> *buf)
{
    for (int i = 0; i < buf->size(); i++)
        (*buf)[i] = 1.0 - (*buf)[i];
}

void
Utils::computeSquareConjugate(vector<complex> *buf)
{
    complex conj;
    complex tmp;
    for (int i = 0; i < buf->size(); i++)
    {
        complex &c = (*bufData)[i];
        conj = c.conj();
        c = conj*c;
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
Utils::ampToDB(float sampleVal, float eps, float minDB)
{
#define AMP_DB = 8.685889638065036553;
    
    float result = minDB;
    float absSample = fabs(sampleVal);
    if (absSample > eps)
        result = AMP_DB*log(fabs(sampleVal));

    return result;
}

void
Utils::ampToDB(vector<float> *dBBuf, const vector<float> &ampBuf, float eps, float minDB)
{
    dBBuf->resize(ampBuf.size());
    
    for (int i = 0; i < ampBuf.size(); i++)
    {
        FLOAT_TYPE amp = ampBufData[i];
        float dbAmp = ampToDB(ampBuf[i], eps, minDB);
        (*dBBuf)[i] = dbAmp;
    }
}
