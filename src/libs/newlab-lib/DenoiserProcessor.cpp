#include "WienerSoftMasking.h"
#include "Utils.h"
#include "Defines.h"
#include "DenoiserProcessor.h"


#define DENOISER_MIN_DB -119.0
#define DENOISER_MAX_DB 10.0

#define USE_RESIDUAL_DENOISE 1
#define RESIDUAL_DENOISE_EPS 1e-15
#define RES_NOISE_HISTORY_SIZE 5

// Process the line #2, so we are in the center of the kernel window
#define RES_NOISE_LINE_NUM 2

#define NOISE_CURVE_SMOOTH_COEFF 0.99

#define DEFAULT_VALUE_SIGNAL 0.0

// 4 gives less gating, but a few musical noise remaining
//#define SOFT_MASKING_HISTO_SIZE 4

// 8 gives more gating, but less musical noise remaining
#define SOFT_MASKING_HISTO_SIZE 8

#define THRESHOLD_COEFF 1000.0


DenoiserProcessor::DenoiserProcessor(int bufferSize, int overlap, float threshold)
: _threshold(threshold)
{
    _bufferSize = bufferSize;
    _overlap = overlap;
    
#if USE_AUTO_RES_NOISE
    _softMasking = NULL;
#endif
    
    _resNoiseThrs = 0.0;
#if USE_AUTO_RES_NOISE
    _autoResNoise = true;
#endif
    
    // Noise capture
    _isBuildingNoiseStatistics = false;
    
#if USE_AUTO_RES_NOISE
    _softMasking = new WienerSoftMasking(bufferSize, overlap,
                                         SOFT_MASKING_HISTO_SIZE);
#endif
    
    resetResNoiseHistory();
}

DenoiserProcessor::~DenoiserProcessor()
{
#if USE_AUTO_RES_NOISE
    if (_softMasking != NULL)
        delete _softMasking;
#endif
}

void
DenoiserProcessor::reset(int bufferSize, int overlap, float sampleRate)
{
    _bufferSize = bufferSize;
    _overlap = overlap;
    _sampleRate = sampleRate;
    
    resampleNoiseCurve();
    
    resetResNoiseHistory();
    
#if USE_AUTO_RES_NOISE
    _softMasking->reset(bufferSize, overlap);
#endif
}

void
DenoiserProcessor::setOverlap(int overlap)
{
    _overlap = overlap;

    reset(_bufferSize, _overlap, _sampleRate);
}

void
DenoiserProcessor::setThreshold(float threshold)
{
    _threshold = threshold;
}

void
DenoiserProcessor::processFFT(vector<complex<float> > *ioBuffer)
{
    // Add noise statistics
    if (_isBuildingNoiseStatistics)
        addNoiseStatistics(*ioBuffer);
    
    vector<float> &sigMagns = _tmpBuf0;
    vector<float> &sigPhases = _tmpBuf1;
    Utils::complexToMagnPhase(&sigMagns, &sigPhases, *ioBuffer);
    
    _signalBuf = sigMagns;
    
    vector<float> &noiseMagns = _tmpBuf2;
    noiseMagns = _noiseCurve;

    if (noiseMagns.size() != sigMagns.size())
        // We havn't defined a noise curve yet
    {
        // Define noise magns as 0, but with the good size
        noiseMagns.resize(sigMagns.size());
        Utils::fillZero(&noiseMagns);
    }
    
    if (!_isBuildingNoiseStatistics && (_noiseCurve.size() == ioBuffer->size()))
        threshold(&sigMagns, &noiseMagns);
    
#if USE_RESIDUAL_DENOISE
    // Keep the possibility to not use residual denoise
    
    // ResidualDenoise introduces a latency, so we must make the noise signal pass
    // there to keep the synchronization
    if (!_isBuildingNoiseStatistics)
        // To eliminate residual noise
        residualDenoise(&sigMagns, &noiseMagns, &sigPhases);
#endif
    
    vector<float> &noisePhases = _tmpBuf3;
    noisePhases = sigPhases;
    
#if USE_AUTO_RES_NOISE
    if (!_isBuildingNoiseStatistics)
        autoResidualDenoise(&sigMagns, &sigPhases, noiseMagns);
#endif
       
    Utils::magnPhaseToComplex(ioBuffer, sigMagns, sigPhases);
    
    _noiseBuf = noiseMagns;
}

void
DenoiserProcessor::getSignalBuffer(vector<float> *ioBuffer)
{
    *ioBuffer = _signalBuf;
}

void
DenoiserProcessor::getNoiseBuffer(vector<float> *ioBuffer)
{
    *ioBuffer = _noiseBuf;
}

void
DenoiserProcessor::setBuildingNoiseStatistics(bool flag)
{    
    if (flag && !_isBuildingNoiseStatistics)
    {
        _noiseCurve.resize(_bufferSize);
        for (int i = 0; i < _noiseCurve.size(); i++)
            _noiseCurve[i] = DEFAULT_VALUE_SIGNAL;
    }

    _isBuildingNoiseStatistics = flag;
}

void
DenoiserProcessor::addNoiseStatistics(const vector<complex<float> > &buf)
{
    // Add the sample to temp noise curve
    vector<float> &noiseCurve = _tmpBuf4;
    noiseCurve.resize(buf.size());
    
    for (int i = 0; i < buf.size(); i++)
    {
        float magn = abs(buf[i]);
        
        noiseCurve[i] = magn;
    }

    for (int i = 0; i < _noiseCurve.size(); i++)
        _noiseCurve[i] =
            NOISE_CURVE_SMOOTH_COEFF*_noiseCurve[i] + (1.0 - NOISE_CURVE_SMOOTH_COEFF)*noiseCurve[i];
    
    _nativeNoiseCurve = _noiseCurve;
}

void
DenoiserProcessor::getNoiseCurve(vector<float> *noiseCurve)
{
    *noiseCurve = _noiseCurve;
}

void
DenoiserProcessor::setNoiseCurve(const vector<float> &noiseCurve)
{
    _noiseCurve = noiseCurve;
}

void
DenoiserProcessor::getNativeNoiseCurve(vector<float> *noiseCurve)
{
    *noiseCurve = _nativeNoiseCurve;
}

void
DenoiserProcessor::setNativeNoiseCurve(const vector<float> &noiseCurve)
{
    _nativeNoiseCurve = noiseCurve;
    
    resampleNoiseCurve();
}

void
DenoiserProcessor::setResNoiseThrs(float threshold)
{
    _resNoiseThrs = threshold;
}

#if USE_AUTO_RES_NOISE
void
DenoiserProcessor::setAutoResNoise(bool flag)
{   
    _autoResNoise = flag;
    
    if (_softMasking != NULL)
        _softMasking->setProcessingEnabled(_autoResNoise);
}
#endif

int
DenoiserProcessor::getLatency()
{
    int latency = 0;

    // Soft masking
    if (_autoResNoise)
    {
        if (_softMasking != NULL)
            latency = _softMasking->getLatency();
    }
    else // Res noise
        latency = RES_NOISE_LINE_NUM*(_bufferSize - 1)*2/_overlap;
    
    return latency;
}

void
DenoiserProcessor::resetResNoiseHistory()
{
    _historyFftBufs.unfreeze();
    _historyFftBufs.clear();

    _historyFftNoiseBufs.unfreeze();
    _historyFftNoiseBufs.clear();

    _historyPhases.unfreeze();
    _historyPhases.clear();
    
    vector<float> &zeroBuf = _tmpBuf5;
    zeroBuf.resize(_bufferSize);
    Utils::fillZero(&zeroBuf);
    
    for (int i = 0; i < RES_NOISE_HISTORY_SIZE; i++)
    {
        _historyFftBufs.push_back(zeroBuf);
        _historyFftNoiseBufs.push_back(zeroBuf);
        _historyPhases.push_back(zeroBuf);
    }
}

void
DenoiserProcessor::residualDenoise(vector<float> *signalBuffer,
                                   vector<float> *noiseBuffer,
                                   vector<float> *phases)
{
    // Make an history which represents the spectrum of the signal
    // Then filter noise by a simple 2d filter, to suppress the residual noise
    
    // Fill the queue with signal buffer
    if (_historyFftBufs.size() != RES_NOISE_HISTORY_SIZE)
    {
        _historyFftBufs.push_back(*signalBuffer);
        if (_historyFftBufs.size() > RES_NOISE_HISTORY_SIZE)
            _historyFftBufs.pop_front();
        if (_historyFftBufs.size() < RES_NOISE_HISTORY_SIZE)
            return;
    }
    else
    {
        _historyFftBufs.freeze();
        _historyFftBufs.push_pop(*signalBuffer);
    }
    
    // Fill the queue with noise buffer
    if (_historyFftNoiseBufs.size() != RES_NOISE_HISTORY_SIZE)
    {
        _historyFftNoiseBufs.push_back(*noiseBuffer);
        if (_historyFftNoiseBufs.size() > RES_NOISE_HISTORY_SIZE)
            _historyFftNoiseBufs.pop_front();
        if (_historyFftNoiseBufs.size() < RES_NOISE_HISTORY_SIZE)
            return;
    }
    else
    {
        _historyFftNoiseBufs.freeze();
        _historyFftNoiseBufs.push_pop(*noiseBuffer);
    }
    
    // Fill the queue with signal buffer
    if (_historyPhases.size() != RES_NOISE_HISTORY_SIZE)
    {
        _historyPhases.push_back(*phases);
        if (_historyPhases.size() > RES_NOISE_HISTORY_SIZE)
            _historyPhases.pop_front();
        if (_historyPhases.size() < RES_NOISE_HISTORY_SIZE)
            return;
    }
    else
    {
        _historyPhases.freeze();
        _historyPhases.push_pop(*phases);
    }
    
    // For latency
    if ((_resNoiseThrs < RESIDUAL_DENOISE_EPS) && !_autoResNoise)
    {
        *signalBuffer = _historyFftBufs[RES_NOISE_LINE_NUM];
        *phases = _historyPhases[RES_NOISE_LINE_NUM];
        
        *noiseBuffer = _historyFftNoiseBufs[RES_NOISE_LINE_NUM];
        
        return;
    }
    
#if USE_AUTO_RES_NOISE
    // If auto res noise, keep spectrogram history, but do not process
    if (_autoResNoise)        
        return;
#endif
    
    // Prepare for non filtering
    int width = signalBuffer->size();
    
    // In the history, we will take half of the values at each pass
    // This is to avoid shifts due to overlap that is 1/2
    int height = RES_NOISE_HISTORY_SIZE;
    
    samplesHistoryToImage(&_historyFftBufs, &_inputImageFilterChunk);
    

    
    // Prepare the output buffer
    if (_outputImageFilterChunk.size() != width*height)
        _outputImageFilterChunk.resize(width*height);
    
    float *input = _inputImageFilterChunk.data();
    float *output = _outputImageFilterChunk.data();
    
    // Just in case
    for (int i = 0; i < width*height; i++)
        output[i] = 0.0;
    
    // Filter the 2d image
    
    int winSize = 5;
    
    if (_hanningKernel.size() != winSize*winSize)
        makeHanningKernel2D(winSize, &_hanningKernel);
    
    noiseFilter(input, output, width, height, winSize, &_hanningKernel,
                RES_NOISE_LINE_NUM, _resNoiseThrs);
    
    imageLineToSamples(&_outputImageFilterChunk, width, height, RES_NOISE_LINE_NUM,
                       &_historyFftBufs, &_historyPhases,
                       signalBuffer, phases);
    
    // Compute the noise part after residual denoise
    vector<float> &histSignal = _historyFftBufs[RES_NOISE_LINE_NUM];
    const vector<float> &histNoise =
        _historyFftNoiseBufs[RES_NOISE_LINE_NUM];
    
    *phases = _historyPhases[RES_NOISE_LINE_NUM];
    
    *noiseBuffer = histNoise;
    
    extractResidualNoise(&histSignal, signalBuffer, noiseBuffer);
}

void
DenoiserProcessor::autoResidualDenoise(vector<float> *ioSignalMagns,
                                       vector<float> *ioSignalPhases,
                                       const vector<float> &noiseMagns)
{    
    // Recompute the complex buffer here
    // This is more safe than using the original comp buffer,
    // because some other operations may have delayed magns and phases
    // so the comp buffer is not synchronized anymore with the processed magns
    // and phases.
    
    // Reconstruct the original signal (magns)
    vector<float> &originMagns = _tmpBuf6;
    originMagns = *ioSignalMagns;
    Utils::addBuffers(&originMagns, noiseMagns);
    
    // Get the original complex buffer
    vector<complex<float> > &compBufferOrig = _tmpBuf7;
    Utils::magnPhaseToComplex(&compBufferOrig, originMagns, *ioSignalPhases);
    
    // Compute hard masks

    // Signal hard mask
    vector<float> &signalMask = _tmpBuf10;
    signalMask.resize(ioSignalMagns->size());

    int signalMaskSize = signalMask.size();
    float *ioSignalMagnsData = ioSignalMagns->data();
    const float *ioNoiseMagnsData = noiseMagns.data();
    float *signalMaskData = signalMask.data();
    
    for (int i = 0; i < signalMaskSize; i++)
    {
        float resSig = ioSignalMagnsData[i];
        float resNoise = ioNoiseMagnsData[i];
        
        float sum = resSig + resNoise;
        float coeff = 0.0;
        if (sum > NL_EPS)
            coeff = resSig/sum;
        
        signalMaskData[i] = coeff;
    }

    // Noise hard mask
    vector<float> &noiseMask = _tmpBuf9;
    noiseMask = signalMask;
    Utils::computeNormOpposite(&noiseMask);

    // Keep a copy, because ProcessCentered() modifies the input
    vector<complex<float> > &compBufferOrigCopy = _tmpBuf12;
    compBufferOrigCopy = compBufferOrig;
    
    // Signal soft masking
    vector<complex<float> > &softMaskedSignal = _tmpBuf22;
    _softMasking->processCentered(&compBufferOrig,
                                  signalMask,
                                  &softMaskedSignal);

    if (!_autoResNoise)
        // Do not process result, but update SoftMaskingComp obj
        return;
    
    // Recompute the result signal magns and noise magns
    vector<float> &signalPhases = _tmpBuf16;
    Utils::complexToMagnPhase(ioSignalMagns, &signalPhases, softMaskedSignal);
    
    *ioSignalPhases = signalPhases;
}

void
DenoiserProcessor::noiseFilter(float *input, float *output, int width, int height,
                               int winSize, vector<float> *kernel, int lineNum,
                               float threshold)
{
#define MIN_THRESHOLD -200.0
#define MAX_THRESHOLD 0.0

    // Optimization: precompute db
    vector<float> &inputDB = _tmpBuf24;
    inputDB.resize(width*height);
    Utils::ampToDB(inputDB.data(), input, inputDB.size(), 1e-15, (float)DENOISER_MIN_DB);
    float *inputDBData = inputDB.data();
    
    // Process only one line (for optimization)
    for (int j = lineNum; j < lineNum + 1; j++)
    {
        for (int i = 0; i < width; i++)
        {
            float avg = 0.0;
            float sum = 0.0;
            
            // By default, copy the input
            int index0 = i + j*width;
            
            output[index0] = input[index0];
            
            float centerVal = input[index0];
            
            if (centerVal == 0.0)
                // Nothing to test, the value is already 0
                continue;
            
            int halfWinSize = winSize/2;
            
            for (int wi = -halfWinSize; wi <= halfWinSize; wi++)
            {
                for (int wj = -halfWinSize; wj <= halfWinSize; wj++)
                {
                    // When out of bounds, continue instead of round, to avoid taking the middle value
                    int x = i + wi;
                    if (x < 0)
                        continue;
                    if (x >= width)
                        continue;
                    
                    int y = j + wj;
                    if (y < 0)
                        continue;
                    if (y >= height)
                        continue;

                    // Use precomputed db values
                    float val = inputDBData[x + y*width];
                    
                    float kernelVal = 1.0;
                    if (kernel != NULL)
                        kernelVal = (*kernel)[(wi + halfWinSize) + (wj + halfWinSize)*winSize];
                    
                    avg += val*kernelVal;
                    sum += kernelVal;
                }
            }
            
            if (sum > 0.0)
                avg /= sum;
            
            float thrs = threshold*(MAX_THRESHOLD - MIN_THRESHOLD) + MIN_THRESHOLD;
            
            if (avg < thrs)
                output[index0] = 0.0;
        }
    }
}

void
DenoiserProcessor::samplesHistoryToImage(const nl_queue<vector<float> > *hist,
                                         vector<float> *imageChunk)
{
    // Get the image dimensions
    int height = (int)hist->size();
    if (height < 1)
    {
        imageChunk->resize(0);
        
        return;
    }
    
    const vector<float> &hist0 = (*hist)[0];
    int width = hist0.size();
    
    imageChunk->resize(width*height);
    
    // Get the image buffer
    float *imageBuf = imageChunk->data();
    
    // Fill the image
    for (int j = 0; j < height; j++)
        // Time
    {
        const vector<float> &histBuf = (*hist)[j];
        
        for (int i = 0; i < width; i++)
            // Bins
        {
            float magn = histBuf[i];
            
            // Take appropriate scale
            float logMagn = log(1.0 + magn);
            
            imageBuf[i + j*width] = logMagn;
        }
    }
}

void
DenoiserProcessor::imageLineToSamples(const vector<float> *image,
                                      int width,
                                      int height,
                                      int lineNum,
                                      const nl_queue<vector<float> > *hist,
                                      const nl_queue<vector<float> > *phaseHist,
                                      vector<float> *resultBuf,
                                      vector<float> *resultPhases)
{
    if (lineNum >= height)
        return;
    
    if (lineNum >= resultBuf->size())
        return;
    
    const vector<float> &histLine = (*hist)[lineNum];
    const vector<float> &phaseLine = (*phaseHist)[lineNum];
    
    *resultBuf = histLine;
    *resultPhases = phaseLine;
    
    // Process
    for (int i = 0; i < width; i++)
    {
        // Take the most recent
        float logMagn = (*image)[i + width*lineNum];
        
        float newMagn = exp(logMagn) - 1.0;
        if (newMagn < 0.0)
            newMagn = 0.0;
        
        (*resultBuf)[i] = newMagn;
    }
}

void
DenoiserProcessor::extractResidualNoise(const vector<float> *prevSignal,
                                        const vector<float> *signal,
                                        vector<float> *ioNoise)
{
    for (int i = 0; i < ioNoise->size(); i++)
    {
        float prevMagn = (*prevSignal)[i];
        float magn = (*signal)[i];
        float noiseMagn = (*ioNoise)[i];
        
        // Positive difference
        float newMagn = noiseMagn + (prevMagn - magn);
        if (noiseMagn < 0.0)
            noiseMagn = 0.0;
        
        (*ioNoise)[i] = newMagn;
    }
}

// See: http://home.mit.bme.hu/~bako/zaozeng/chapter4.htm
void
DenoiserProcessor::threshold(vector<float> *ioSigMagns,
                             vector<float> *ioNoiseMagns)
{
    if (ioSigMagns->size() != ioNoiseMagns->size())
        return;
    
    vector<float> &thrsNoiseMagns = _tmpBuf19;
    thrsNoiseMagns = *ioNoiseMagns;
    
    applyThresholdValueToNoiseCurve(&thrsNoiseMagns, _threshold);
    
    // Threshold, soft elbow
    for (int i = 0; i < ioSigMagns->size(); i++)
    {
        float magn = (*ioSigMagns)[i];
        float noise = thrsNoiseMagns[i];
        
        float newMagn = (magn + 1.0)/(noise + 1.0) - 1.0;
        if (newMagn < 0.0)
            newMagn = 0.0;
        
        (*ioSigMagns)[i] = newMagn;
        
        float newNoise = magn - newMagn;
        if (newNoise < 0.0)
            newNoise = 0.0;
        
        (*ioNoiseMagns)[i] = newNoise;
    }
}

void
DenoiserProcessor::applyThresholdValueToNoiseCurve(vector<float> *ioNoiseCurve, float threshold)
{
    // Apply threshold not in dB
    float thrs0 = threshold*THRESHOLD_COEFF;
    for (int i = 0; i < ioNoiseCurve->size(); i++)
    {
        float sample = (*ioNoiseCurve)[i];
        
        sample *= thrs0;
        
        (*ioNoiseCurve)[i] = sample;
    }
}

void
DenoiserProcessor::resampleNoiseCurve()
{
    _noiseCurve = _nativeNoiseCurve;
    
    Utils::resizeFillZeros(&_noiseCurve, _bufferSize);
}

void
DenoiserProcessor::makeHanningKernel2D(int size, vector<float> *result)
{
    result->resize(size*size);
    
    float *hanning = result->data();
    
	float maxDist = size/2 + 1;
    
    for (int j = -size/2; j <= size/2; j++)
        for (int i = -size/2; i <= size/2; i++)
        {
            float dist = std::sqrt((float)(i*i + j*j));
            if (dist > maxDist)
                continue;
            
            float val = std::cos((float)(0.5*M_PI * dist/maxDist));
            hanning[(i + size/2) + (j + size/2)*size] = val;
        }
}
