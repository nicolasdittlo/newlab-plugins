#include "Defines.h"
#include "OverlapAdd.h"
#include "Utils.h"
#include "TransientLib.h"

#include "TransientShaperProcessor.h"

// Detection + correction
#define TRANSIENTNESS_COEFF 5.0

TransientShapeProcessor::TransientShapeProcessor(float sampleRate)
{
    _sampleRate = sampleRate;
        
    _transLib = new TransientLib();
    
    _precision = 0.0;
    _softHard = 0.0;
    
    _freqAmpRatio = 0.5;
}

TransientShapeProcessor::~TransientShapeProcessor()
{
    delete _transLib;
}

void
TransientShapeProcessor::reset(float sampleRate)
{
    _sampleRate = sampleRate;
}

void
TransientShapeProcessor::setPrecision(float precision)
{
    _precision = precision;
}

void
TransientShapeProcessor::setSoftHard(float softHard)
{
    _softHard = softHard;
}

void
TransientShapeProcessor::setFreqAmpRatio(float ratio)
{
    _freqAmpRatio = ratio;
}

void
TransientShapeProcessor::
processFFT(vector<complex<float> > *ioBuffer)
{    
    // Seems hard to take half, since we work in sample space too...
    
    vector<complex<float> > &fftBuffer = _tmpBuf1;
    fftBuffer = *ioBuffer;
    
    vector<float> &magns = _tmpBuf2;
    vector<float> &phases = _tmpBuf3;
    Utils::complexToMagnPhase(&magns, &phases, fftBuffer);
    
    // Compute the transientness
    vector<float> &transientness = _tmpBuf4;

    // Final: good for 88200Hz + buffer size 4096
    _transLib->computeTransientness(magns, phases,
                                    &_prevPhases,
                                    _freqAmpRatio,
                                    1.0 - _precision,
                                    _sampleRate,
                                    &transientness);
    
    _transientness = transientness;
    Utils::multValue(&_transientness, (float)TRANSIENTNESS_COEFF);
    
    _prevPhases = phases;
}

void
TransientShapeProcessor::processOutSamples(vector<float> *ioBuffer)
{        
    applyTransientness(ioBuffer, _transientness);
}

void
TransientShapeProcessor::
getTransientness(vector<float> *outTransientness)
{
    *outTransientness = _transientness;
}

float
TransientShapeProcessor::computeMaxTransientness()
{
#define MAX_GAIN 50.0
#define MAX_GAIN_CLIP 6.0

    // Just to be sure to not reach exactly 1.0 in the samples
#define FACTOR 0.999
    
    if (std::fabs(_softHard) < NL_EPS)
      return 1.0*FACTOR;
    
    float maxTransDB = -MAX_GAIN_CLIP/_softHard;
    
    float maxTrans = Utils::DBToAmp(maxTransDB);
    
    return maxTrans*FACTOR;
}


void
TransientShapeProcessor::applyTransientness(vector<float> *ioSamples,
                                            const vector<float> &transientness)
{
    if (transientness.size() != ioSamples->size())
        return;
    
    vector<float> &trans = _tmpBuf9;
    trans = transientness;
    
    // Avoid clipping (intelligently)
    float maxTransientness = computeMaxTransientness();
    Utils::clipMax(&trans, maxTransientness);
    
    float gainDB = MAX_GAIN*_softHard;
    
    vector<float> &gainsDB = _tmpBuf10;
    gainsDB = trans;
    Utils::multValue(&gainsDB, gainDB);
    
    vector<float> &gains = _tmpBuf11;
    gains = gainsDB;
    Utils::DBToAmp(&gains);
    
    Utils::multBuffers(ioSamples, gains);
}
