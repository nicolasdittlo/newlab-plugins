#include "WienerSoftMasking.h"
#include "Utils.h"
#include "DenoiserProcessor.h"


#define DENOISER_MIN_DB -119.0
#define DENOISER_MAX_DB 10.0

#define USE_RESIDUAL_DENOISE 1
#define RESIDUAL_DENOISE_EPS 1e-15
#define RES_NOISE_HISTORY_SIZE 5

// Process the line #2, so we are in the center of the kernel window
#define RES_NOISE_LINE_NUM 2

#define NOISE_PATTERN_SMOOTH_COEFF 0.99

#define DEFAULT_VALUE_SIGNAL 0.0

// 4 gives less gating, but a few musical noise remaining
//#define SOFT_MASKING_HISTO_SIZE 4

// 8 gives more gating, but less musical noise remaining
#define SOFT_MASKING_HISTO_SIZE 8

#define THRESHOLD_COEFF 1000.0


DenoiserProcessor::DenoiserProcessor(int bufferSize, int overlap, float threshold)
  _threshold(threshold)
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
    
    ResetResNoiseHistory();
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
    
    ResampleNoisePattern();
    
    ResetResNoiseHistory();
    
#if USE_AUTO_RES_NOISE
    _softMasking->Reset(bufferSize, overlap);
#endif
}

void
DenoiserProcessor::setThreshold(float threshold)
{
    _threshold = threshold;
}

void
DenoiserProcessor::processFft(vector<complex<float> > *ioBuffer)
{
    // Add noise statistics
    if (_isBuildingNoiseStatistics)
        AddNoiseStatistics(ioBuffer);
    
    vector<float> &sigMagns = _tmpBuf0;
    vector<float> &sigPhases = _tmpBuf1;
    Utils::complexToMagnPhase(&sigMagns, &sigPhases, ioBuffer);
    
    _signalBuf = sigMagns;
    
    vector<float> &noiseMagns = _tmpBuf2;
    noiseMagns = _noisePattern;

    if (noiseMagns.size() != sigMagns.size())
        // We havn't defined a noise pattern yet
    {
        // Define noise magns as 0, but with the good size
        noiseMagns.resize(sigMagns.size());
        Utils::fillZero(&noiseMagns);
    }
    
    if (!_isBuildingNoiseStatistics && (_noisePattern.size() == ioBuffer.size()))
        Threshold(&sigMagns, &noiseMagns);
    
#if USE_RESIDUAL_DENOISE
    // Keep the possibility to not use residual denoise
    
    // ResidualDenoise introduces a latency, so we must make the noise signal pass
    // there to keep the synchronization
    if (!_isBuildingNoiseStatistics)
        // To eliminate residual noise
        ResidualDenoise(&sigMagns, &noiseMagns, &sigPhases);
#endif
    
    vector<float> &noisePhases = _tmpBuf3;
    noisePhases = sigPhases;
    
#if USE_AUTO_RES_NOISE
    if (!_isBuildingNoiseStatistics)
        AutoResidualDenoise(&sigMagns, &sigPhases);
#endif
       
    BLUtils::magnPhaseToComplex(&ioBuffer, sigMagns, sigPhases);
    
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
        _noisePattern.resize(_bufferSize);
        for (int i = 0; i < _noisePattern.size(); i++)
            _noisePattern[i] = DEFAULT_VALUE_SIGNAL;
    }

    _isBuildingNoiseStatistics = flag;
}

void
DenoiserProcessor::addNoiseStatistics(const vector<complex<float> > &buf)
{
    // Add the sample to temp noise pattern
    vector<float> &noisePattern = _tmpBuf4;
    noisePattern.Resize(buf.size());
    
    for (int i = 0; i < buf.size(); i++)
    {
        float magn = abs(buf[i]);
        
        noisePattern[i] = magn;
    }

    for (int i = 0; i < _noisePattern.size(); i++)
        _noisePattern[i] =
            NOISE_PATTERN_SMOOTH_COEFF*_noisePattern[i] + (1.0 - NOISE_PATTERN_SMOOTH_COEFF)*noisePattern[i];
    
    _nativeNoisePattern = _noisePattern;
}

void
DenoiserProcessor::getNoisePattern(vector<float> *noisePattern)
{
    *noisePattern = _noisePattern;
}

void
DenoiserProcessor::setNoisePattern(const vector<float> &noisePattern)
{
    _noisePattern = noisePattern;
}

void
DenoiserProcessor::getNativeNoisePattern(vector<float> *noisePattern)
{
    *noisePattern = _nativeNoisePattern;
}

void
DenoiserProcessor::setNativeNoisePattern(const vector<float> &noisePattern)
{
    _nativeNoisePattern = noisePattern;
    
    ResampleNoisePattern();
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
        _softMasking->SetProcessingEnabled(_autoResNoise);
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
            latency = _softMasking->GetLatency();
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
    zeroBuf.Resize(_bufferSize);
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
    
    // Prepare for non filtering
    int width = signalBuffer->size();
    
    // In the history, we will take half of the values at each pass
    // This is to avoid shifts due to overlap that is 1/2
    int height = RES_NOISE_HISTORY_SIZE;
    
    samplesHistoryToImage(&_historyFftBufs, &_inputImageFilterChunk);
    

    
    // Prepare the output buffer
    if (_outputImageFilterChunk.size() != width*height)
        _outputImageFilterChunk.Resize(width*height);
    
    float *input = _inputImageFilterChunk;
    float *output = _outputImageFilterChunk;
    
    // Just in case
    for (int i = 0; i < width*height; i++)
        output[i] = 0.0;
    
    // Filter the 2d image
    
    int winSize = 5;
    
    if (_hanningKernel.size() != winSize*winSize)
        MakeHanningKernel2D(winSize, &_hanningKernel);
    
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
                                       vector<float> *ioSignalPhases)
{    
    // Recompute the complex buffer here
    // This is more safe than using the original comp buffer,
    // because some other operations may have delayed magns and phases
    // so the comp buffer is not synchronized anymore with the processed magns
    // and phases.
    
    // Reconstruct the original signal (magns)
    vector<float> &originMagns = _tmpBuf6;
    originMagns = *ioSignalMagns;
    Utils::addBuffer(&originMagns, *ioNoiseMagns);
    
    // Get the original complex buffer
    vector<complex<float> > &compBufferOrig = _tmpBuf7;
    Utils::magnPhaseToComplex(&compBufferOrig, originMagns, *ioSignalPhases);
    
    // Compute hard masks

    // Signal hard mask
    vector<float> &signalMask = _tmpBuf10;
    signalMask.Resize(ioSignalMagns->size());

    int signalMaskSize = signalMask.size();
    float *ioSignalMagnsData = ioSignalMagns->data();
    float *ioNoiseMagnsData = ioNoiseMagns->data();
    float *signalMaskData = signalMask;
    
    for (int i = 0; i < signalMaskSize; i++)
    {
        float resSig = ioSignalMagnsData[i];
        float resNoise = ioNoiseMagnsData[i];
        
        float sum = resSig + resNoise;
        float coeff = 0.0;
        if (sum > 1e-15)
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
    _softMasking->ProcessCentered(&compBufferOrig,
                                  signalMask,
                                  &softMaskedSignal);

    if (!_autoResNoise)
        // Do not process result, but update SoftMaskingComp obj
        return;
    
    // Recompute the result signal magns and noise magns
    vector<float> &signalPhases = _tmpBuf16;
    Utils::complexToMagnPhase(ioSignalMagns, &signalPhases, softMaskedSignal);
    
    *ioSignalPhases = signalPhases;
    *ioNoisePhases = noisePhases;
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
    inputDB.Resize(width*height);
    Utils::ampToDB(inputDB, input, inputDB.size(), 1e-15, (float)DENOISER_MIN_DB);
    float *inputDBData = inputDB;
    
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
DenoiserProcessor::samplesHistoryToImage(const bl_queue<vector<float> > *hist,
                                         vector<float> *imageChunk)
{
    // Get the image dimensions
    int height = (int)hist->size();
    if (height < 1)
    {
        imageChunk->Resize(0);
        
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
                                      const bl_queue<vector<float> > *hist,
                                      const bl_queue<vector<float> > *phaseHist,
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
    
    ApplyThresholdValueToNoiseCurve(&thrsNoiseMagns, _threshold);
    
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
DenoiserProcessor::resampleNoisePattern()
{
    _noisePattern = _nativeNoisePattern;
    
    Utils::resizeFillZeros(&_noisePattern, _bufferSize);
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
