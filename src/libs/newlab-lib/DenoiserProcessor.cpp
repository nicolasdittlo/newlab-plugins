#include <SoftMaskingComp4.h>
#include <SmoothAvgHistogram.h>

#include <BLUtils.h>
#include <BLUtilsComp.h>
#include <BLUtilsFft.h>
#include <BLUtilsMath.h>
#include <BLUtilsPlug.h>

#include <BLDebug.h>

#include <Window.h>

#include "DenoiserProcessor.h"

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

#define FIX_RES_NOISE_LATENCY_SYNCHRO 1

#define LATENCY_AUTO_RES_NOISE_OPTIM 1

#define FIX_RES_NOISE_KERNEL 1

// Apply soft masking on extracted noise too?
// (or simply compute the difference at the end?)
#define AUTO_RES_NOISE_SOFT_MASK_NOISE 0

#define THRESHOLD_COEFF 1000.0

// Avoid computing everything when auto res noise is disabled.
// In this case, we only need the history, not computing expectation
// each time
#define OPTIM_COMP_MASK_BYPASS 1

#define DENOISER_BUFFER_SIZE 2048

DenoiserObj::DenoiserObj(Plugin *iPlug,
                         int bufferSize, int overlapping, int freqRes,
                         BL_FLOAT threshold, bool processNoise)
: ProcessObj(bufferSize),
  mPlug(iPlug), mThreshold(threshold), mProcessNoise(processNoise)
{
    mOverlapping = overlapping;
    
    mTwinNoiseObj = NULL;
    
    mAvgHistoNoisePattern = NULL;
    
#if USE_AUTO_RES_NOISE
    for (int i = 0; i < 2; i++)
        mSoftMaskingComps[i] = NULL;
#endif
    
    if (mProcessNoise)
        return;
    
    mResNoiseThrs = 0.0;
#if USE_AUTO_RES_NOISE
    mAutoResNoise = true;
#endif
    
    // Noise capture
    mIsBuildingNoiseStatistics = false;
    mNumNoiseStatistics = 0;
    
    // Must set a big coeff to be very smooth !
    mAvgHistoNoisePattern = new SmoothAvgHistogram(bufferSize/2,
                                                   NOISE_PATTERN_SMOOTH_COEFF,
                                                   DEFAULT_VALUE_SIGNAL);
    
#if USE_AUTO_RES_NOISE
    for (int i = 0; i < 2; i++)
        mSoftMaskingComps[i] = new SoftMaskingComp4(bufferSize, overlapping,
                                                    SOFT_MASKING_HISTO_SIZE);
#endif
    
    ResetResNoiseHistory();
}

DenoiserObj::~DenoiserObj()
{
    if (mAvgHistoNoisePattern != NULL)
        delete mAvgHistoNoisePattern;
    
#if USE_AUTO_RES_NOISE
    for (int i = 0; i < 2; i++)
    {
        if (mSoftMaskingComps[i] != NULL)
            delete mSoftMaskingComps[i];
    }
#endif
}

void
DenoiserObj::Reset(int bufferSize, int overlapping,
                   int freqRes, BL_FLOAT sampleRate)
{
    ProcessObj::Reset(bufferSize, overlapping, freqRes, sampleRate);
    
    if (mProcessNoise)
        return;
    
#if USE_VARIABLE_BUFFER_SIZE
    // Re-create the noise profile smoother,
    // because the buffer size may have changed
    if (mAvgHistoNoisePattern != NULL)
        delete mAvgHistoNoisePattern;
    mAvgHistoNoisePattern = new SmoothAvgHistogram(bufferSize/2,
                                                   NOISE_PATTERN_SMOOTH_COEFF,
                                                   DEFAULT_VALUE_SIGNAL);
    ResampleNoisePattern();
#endif
    
    ResetResNoiseHistory();
    
#if USE_AUTO_RES_NOISE
    for (int i = 0; i < 2; i++)
        mSoftMaskingComps[i]->Reset(bufferSize, overlapping);
#endif
}

void
DenoiserObj::SetThreshold(BL_FLOAT threshold)
{
    if (mProcessNoise)
        return;
    
    mThreshold = threshold;
}

void
DenoiserObj::ProcessFftBuffer(WDL_TypedBuf<WDL_FFT_COMPLEX> *ioBuffer0,
                              const WDL_TypedBuf<WDL_FFT_COMPLEX> *scBuffer)
{
    if (mProcessNoise)
        // This is a twin noise obj
    {
        WDL_TypedBuf<WDL_FFT_COMPLEX> &ioBuffer = mTmpBuf20;
        BLUtils::TakeHalf(*ioBuffer0, &ioBuffer);
        
        BLUtilsComp::MagnPhaseToComplex(&ioBuffer, mNoiseBuf, mNoiseBufPhases);
        
        BLUtilsFft::FillSecondFftHalf(ioBuffer, ioBuffer0);
        
        return;
    }

    WDL_TypedBuf<WDL_FFT_COMPLEX> &ioBuffer = mTmpBuf21;
    BLUtils::TakeHalf(*ioBuffer0, &ioBuffer);
    
    // Add noise statistics
    if (mIsBuildingNoiseStatistics)
        AddNoiseStatistics(ioBuffer);
    
    WDL_TypedBuf<BL_FLOAT> &sigMagns = mTmpBuf0;
    WDL_TypedBuf<BL_FLOAT> &sigPhases = mTmpBuf1;
    BLUtilsComp::ComplexToMagnPhase(&sigMagns, &sigPhases, ioBuffer);
    
    mSignalBuf = sigMagns;
    
    WDL_TypedBuf<BL_FLOAT> &noiseMagns = mTmpBuf2;
    noiseMagns = mNoisePattern;

    if (noiseMagns.GetSize() != sigMagns.GetSize())
        // We havn't defined a noise pattern yet
    {
        // Define noise magns as 0, but with the good size
        // So they won't have size == 0 (to avoid crash)
        noiseMagns.Resize(sigMagns.GetSize());
        BLUtils::FillAllZero(&noiseMagns);
    }
    
    if (!mIsBuildingNoiseStatistics &&
        (mNoisePattern.GetSize() == ioBuffer.GetSize()))
    {
        Threshold(&sigMagns, &noiseMagns);
    }
    
#if USE_RESIDUAL_DENOISE
    // Keep the possibility to not residual denoise
    // (because it is time consumming and sometime useless)
    //if (mResNoiseThrs > RESIDUAL_DENOISE_EPS) // Do not test here (to get constant latency)
    //{

    // ResidualDenoise introduces a latency, so we must make the noise signal pass
    // there, to keep the synchronization
    // Otherwise, when we re-inject noise in the final signal, there will be an offset when earing
    if (!mIsBuildingNoiseStatistics)
        // To eliminate residual noise
        ResidualDenoise(&sigMagns, &noiseMagns, &sigPhases);
#endif
    
    WDL_TypedBuf<BL_FLOAT> &noisePhases = mTmpBuf3;
    noisePhases = sigPhases;
    
#if USE_AUTO_RES_NOISE
    if (!mIsBuildingNoiseStatistics)
        AutoResidualDenoise(&sigMagns, &noiseMagns,
                            &sigPhases, &noisePhases);
#endif
    
    if (!mProcessNoise)
    {
        BLUtilsComp::MagnPhaseToComplex(&ioBuffer, sigMagns, sigPhases);
    }
    else
    {
        BLUtilsComp::MagnPhaseToComplex(&ioBuffer, noiseMagns, noisePhases);
    }
    
    mNoiseBuf = noiseMagns;
    
    if (mTwinNoiseObj != NULL)
    {
        mTwinNoiseObj->SetTwinNoiseBuf(noiseMagns, noisePhases);
    }
    
    BLUtilsFft::FillSecondFftHalf(ioBuffer, ioBuffer0);
}

void
DenoiserObj::GetSignalBuffer(WDL_TypedBuf<BL_FLOAT> *ioBuffer)
{
    if (mProcessNoise)
        return;
    
    *ioBuffer = mSignalBuf;
}

void
DenoiserObj::GetNoiseBuffer(WDL_TypedBuf<BL_FLOAT> *ioBuffer)
{
    *ioBuffer = mNoiseBuf;
}

void
DenoiserObj::SetBuildingNoiseStatistics(bool flag)
{
    if (mProcessNoise)
        return;
    
    mIsBuildingNoiseStatistics = flag;
    
    if (flag)
    {
        mNoisePattern.Resize(0);
    }
}

void
DenoiserObj::AddNoiseStatistics(const WDL_TypedBuf<WDL_FFT_COMPLEX> &buf)
{
    if (mProcessNoise)
        return;
    
    // Add the sample to temp noise pattern
    WDL_TypedBuf<BL_FLOAT> &noisePattern = mTmpBuf4;
    noisePattern.Resize(buf.GetSize());
    
    // Only half of the buffer is useful
    for (int i = 0; i < buf.GetSize(); i++)
    {
        BL_FLOAT magn = COMP_MAGN(buf.Get()[i]);
        
        noisePattern.Get()[i] = magn;
    }
    
    mAvgHistoNoisePattern->AddValues(noisePattern);
    mAvgHistoNoisePattern->GetValues(&mNoisePattern);
    
#if USE_VARIABLE_BUFFER_SIZE
    mNativeNoisePattern = mNoisePattern;
#endif
}

void
DenoiserObj::GetNoisePattern(WDL_TypedBuf<BL_FLOAT> *noisePattern)
{
    if (mProcessNoise)
        return;
    
    *noisePattern = mNoisePattern;
}

void
DenoiserObj::SetNoisePattern(const WDL_TypedBuf<BL_FLOAT> &noisePattern)
{
    if (mProcessNoise)
        return;
    
    mNoisePattern = noisePattern;
}

#if USE_VARIABLE_BUFFER_SIZE
void
DenoiserObj::GetNativeNoisePattern(WDL_TypedBuf<BL_FLOAT> *noisePattern)
{
    if (mProcessNoise)
        return;
    
    *noisePattern = mNativeNoisePattern;
}

void
DenoiserObj::SetNativeNoisePattern(const WDL_TypedBuf<BL_FLOAT> &noisePattern)
{
    if (mProcessNoise)
        return;
    
    mNativeNoisePattern = noisePattern;
    
    ResampleNoisePattern();
}
#endif

void
DenoiserObj::SetResNoiseThrs(BL_FLOAT threshold)
{
    mResNoiseThrs = threshold;
}

#if USE_AUTO_RES_NOISE
void
DenoiserObj::SetAutoResNoise(bool autoResNoiseFlag)
{   
    mAutoResNoise = autoResNoiseFlag;
    
#if OPTIM_COMP_MASK_BYPASS
    for (int i = 0; i < 2; i++)
    {
        if (mSoftMaskingComps[i] != NULL)
        {
            mSoftMaskingComps[i]->SetProcessingEnabled(mAutoResNoise);
        }
    }
#endif
}
#endif

void
DenoiserObj::SetTwinNoiseObj(DenoiserObj *twinObj)
{
    mTwinNoiseObj = twinObj;
}

int
DenoiserObj::GetLatency()
{
    int latency = 0;

    // Warning: with the actual code, tha latencies doesn't seem to
    // sum to the final latency.
    // Instead, take one or the other latency

    // Soft masking
    if (mAutoResNoise)
    {
        if (mSoftMaskingComps[0] != NULL)
            latency = mSoftMaskingComps[0]->GetLatency();
    }
    else // Res noise
    {
        latency = RES_NOISE_LINE_NUM*mBufferSize/mOverlapping;
    }
    
    return latency;
}

void
DenoiserObj::SetTwinNoiseBuf(const WDL_TypedBuf<BL_FLOAT> &noiseBuf,
                             const WDL_TypedBuf<BL_FLOAT> &noiseBufPhases)
{
    mNoiseBuf = noiseBuf;
    mNoiseBufPhases = noiseBufPhases;
}

void
DenoiserObj::ResetResNoiseHistory()
{
    if (mProcessNoise)
        return;

    mHistoryFftBufs.unfreeze();
    mHistoryFftBufs.clear();

    mHistoryFftNoiseBufs.unfreeze();
    mHistoryFftNoiseBufs.clear();

    mHistoryPhases.unfreeze();
    mHistoryPhases.clear();
    
    WDL_TypedBuf<BL_FLOAT> &zeroBuf = mTmpBuf5;
    zeroBuf.Resize(mBufferSize/2);
    BLUtils::FillAllZero(&zeroBuf);
    
    for (int i = 0; i < RES_NOISE_HISTORY_SIZE; i++)
    {
        mHistoryFftBufs.push_back(zeroBuf);
        mHistoryFftNoiseBufs.push_back(zeroBuf);
        mHistoryPhases.push_back(zeroBuf);
    }
}

void
DenoiserObj::ResidualDenoise(WDL_TypedBuf<BL_FLOAT> *signalBuffer,
                             WDL_TypedBuf<BL_FLOAT> *noiseBuffer,
                             WDL_TypedBuf<BL_FLOAT> *phases)
{
    if (mProcessNoise)
        return;
    
    // Make an history which represents the spectrum of the signal
    // Then filter noise by a simple 2d filter, to suppress the residual noise
    
    // Fill the queue with signal buffer
    if (mHistoryFftBufs.size() != RES_NOISE_HISTORY_SIZE)
    {
        //mHistoryFftBufs.push_front(*signalBuffer);
        mHistoryFftBufs.push_back(*signalBuffer);
        if (mHistoryFftBufs.size() > RES_NOISE_HISTORY_SIZE)
            //mHistoryFftBufs.pop_back();
            mHistoryFftBufs.pop_front();
        if (mHistoryFftBufs.size() < RES_NOISE_HISTORY_SIZE)
            return;
    }
    else
    {
        mHistoryFftBufs.freeze();
        mHistoryFftBufs.push_pop(*signalBuffer);
    }
    
    // Fill the queue with noise buffer
    if (mHistoryFftNoiseBufs.size() != RES_NOISE_HISTORY_SIZE)
    {
        mHistoryFftNoiseBufs.push_back(*noiseBuffer);
        if (mHistoryFftNoiseBufs.size() > RES_NOISE_HISTORY_SIZE)
            mHistoryFftNoiseBufs.pop_front();
        if (mHistoryFftNoiseBufs.size() < RES_NOISE_HISTORY_SIZE)
            return;
    }
    else
    {
        mHistoryFftNoiseBufs.freeze();
        mHistoryFftNoiseBufs.push_pop(*noiseBuffer);
    }
    
    // Fill the queue with signal buffer
    if (mHistoryPhases.size() != RES_NOISE_HISTORY_SIZE)
    {
        mHistoryPhases.push_back(*phases);
        if (mHistoryPhases.size() > RES_NOISE_HISTORY_SIZE)
            mHistoryPhases.pop_front();
        if (mHistoryPhases.size() < RES_NOISE_HISTORY_SIZE)
            return;
    }
    else
    {
        mHistoryPhases.freeze();
        mHistoryPhases.push_pop(*phases);
    }
    
    // For FftObj and latency
    if ((mResNoiseThrs < RESIDUAL_DENOISE_EPS) && !mAutoResNoise)
    {
        *signalBuffer = mHistoryFftBufs[RES_NOISE_LINE_NUM];
        *phases = mHistoryPhases[RES_NOISE_LINE_NUM];
        
#if FIX_RES_NOISE_LATENCY_SYNCHRO
        *noiseBuffer = mHistoryFftNoiseBufs[RES_NOISE_LINE_NUM];
#endif
        
        return;
    }
    
#if USE_AUTO_RES_NOISE
    // If auto res noise, keep spectrogram history, but do not process
    if (mAutoResNoise)
    {
#if !LATENCY_AUTO_RES_NOISE_OPTIM
        *signalBuffer = mHistoryFftBufs[RES_NOISE_LINE_NUM];
        *phases = mHistoryPhases[RES_NOISE_LINE_NUM];
        
#if FIX_RES_NOISE_LATENCY_SYNCHRO
        *noiseBuffer = mHistoryFftNoiseBufs[RES_NOISE_LINE_NUM];
#endif
#endif
        
        return;
    }
#endif
    
    // Prepare for non filtering
    int width = signalBuffer->GetSize();
    
    // In the history, we will take half of the values at each pass
    // This is to avoid shifts due to overlap that is 1/2
    int height = RES_NOISE_HISTORY_SIZE;
    
    SamplesHistoryToImage(&mHistoryFftBufs, &mInputImageFilterChunk);
    
    // Process the signal each time
    // Either we will keep the signal, or the noise at the end
    
    // Prepare the output buffer
    if (mOutputImageFilterChunk.GetSize() != width*height)
        mOutputImageFilterChunk.Resize(width*height);
    
    BL_FLOAT *input = mInputImageFilterChunk.Get();
    BL_FLOAT *output = mOutputImageFilterChunk.Get();
    
    // Just in case
    for (int i = 0; i < width*height; i++)
        output[i] = 0.0;
    
    // Filter the 2d image
    
    int winSize = 5;
    
    if (mHanningKernel.GetSize() != winSize*winSize)
    {
#if !FIX_RES_NOISE_KERNEL
        Window::MakeHanningKernel(winSize, &mHanningKernel);
#else
        Window::MakeHanningKernel2(winSize, &mHanningKernel);
#endif
    }
    
    NoiseFilter(input, output, width, height, winSize, &mHanningKernel,
                RES_NOISE_LINE_NUM, mResNoiseThrs);
    
    ImageLineToSamples(&mOutputImageFilterChunk, width, height, RES_NOISE_LINE_NUM,
                       &mHistoryFftBufs, &mHistoryPhases,
                       signalBuffer, phases);
    
    // Compute the noise part after residual denoise
    WDL_TypedBuf<BL_FLOAT> &histSignal = mHistoryFftBufs[RES_NOISE_LINE_NUM];
    const WDL_TypedBuf<BL_FLOAT> &histNoise =
        mHistoryFftNoiseBufs[RES_NOISE_LINE_NUM];
    
    // NEW: addee when Fft15 and latency
    *phases = mHistoryPhases[RES_NOISE_LINE_NUM];
    
    *noiseBuffer = histNoise;
    
    ExtractResidualNoise(&histSignal, signalBuffer, noiseBuffer);
}

void
DenoiserObj::AutoResidualDenoise(WDL_TypedBuf<BL_FLOAT> *ioSignalMagns,
                                 WDL_TypedBuf<BL_FLOAT> *ioNoiseMagns,
                                 WDL_TypedBuf<BL_FLOAT> *ioSignalPhases,
                                 WDL_TypedBuf<BL_FLOAT> *ioNoisePhases)
{    
    if (mProcessNoise)
        return;
    
    // Recompute the complex buffer here
    // This is more safe than using the original comp buffer,
    // because some other operations may have delayed magna and phases
    // so the comp buffer is not synchronized anymore with the processed magns
    // and phases.
    
    // Reconstruct the original signal (magns)
    WDL_TypedBuf<BL_FLOAT> &originMagns = mTmpBuf6;
    originMagns = *ioSignalMagns;
    BLUtils::AddValues(&originMagns, *ioNoiseMagns);
    
    // Get the original complex buffer
    WDL_TypedBuf<WDL_FFT_COMPLEX> &compBufferOrig = mTmpBuf7;
    BLUtilsComp::MagnPhaseToComplex(&compBufferOrig, originMagns, *ioSignalPhases);
    
    // Compute hard masks

    // Signal hard mask
    WDL_TypedBuf<BL_FLOAT> &signalMask = mTmpBuf10;
    signalMask.Resize(ioSignalMagns->GetSize());

    int signalMaskSize = signalMask.GetSize();
    BL_FLOAT *ioSignalMagnsData = ioSignalMagns->Get();
    BL_FLOAT *ioNoiseMagnsData = ioNoiseMagns->Get();
    BL_FLOAT *signalMaskData = signalMask.Get();
    
    for (int i = 0; i < signalMaskSize; i++)
    {
        BL_FLOAT resSig = ioSignalMagnsData[i];
        BL_FLOAT resNoise = ioNoiseMagnsData[i];
        
        BL_FLOAT sum = resSig + resNoise;
        BL_FLOAT coeff = 0.0;
        if (sum > BL_EPS)
            coeff = resSig/sum;
        
        signalMaskData[i] = coeff;
    }

    // Noise hard mask
    WDL_TypedBuf<BL_FLOAT> &noiseMask = mTmpBuf9;
    noiseMask = signalMask;
    BLUtils::ComputeOpposite(&noiseMask);

    // Keep a copy, because ProcessCentered() modifies the input
    WDL_TypedBuf<WDL_FFT_COMPLEX> &compBufferOrigCopy = mTmpBuf12;
    compBufferOrigCopy = compBufferOrig;
    
    // Signal soft masking
    WDL_TypedBuf<WDL_FFT_COMPLEX> &softMaskedSignal = mTmpBuf22;
    mSoftMaskingComps[0]->ProcessCentered(&compBufferOrig,
                                          signalMask,
                                          &softMaskedSignal);

    // Noise soft masking
    // Use buffer copy, to avoid shifting 2 times
    WDL_TypedBuf<WDL_FFT_COMPLEX> &softMaskedNoise = mTmpBuf23;
    mSoftMaskingComps[1]->ProcessCentered(&compBufferOrigCopy,
                                          noiseMask,
                                          &softMaskedNoise);
    
    if (!mAutoResNoise)
        // Do not process result, but update SoftMaskingComp obj
    {
#if !LATENCY_AUTO_RES_NOISE_OPTIM
        // Update, for latency
        BLUtils::ComplexToMagnPhase(ioSignalMagns, ioSignalPhases, signalBufferComp);
        BLUtils::ComplexToMagnPhase(ioNoiseMagns, ioNoisePhases, noiseBufferComp);
#endif
        
        return;
    }
    
    // Recompute the result signal magns and noise magns
    WDL_TypedBuf<BL_FLOAT> &signalPhases = mTmpBuf16;
    BLUtilsComp::ComplexToMagnPhase(ioSignalMagns, &signalPhases, softMaskedSignal);
    
    WDL_TypedBuf<BL_FLOAT> &noisePhases = mTmpBuf18;
    BLUtilsComp::ComplexToMagnPhase(ioNoiseMagns, &noisePhases, softMaskedNoise);
    
    *ioSignalPhases = signalPhases;
    *ioNoisePhases = noisePhases;
}
#endif

void
DenoiserObj::NoiseFilter(BL_FLOAT *input, BL_FLOAT *output, int width, int height,
                         int winSize, WDL_TypedBuf<BL_FLOAT> *kernel, int lineNum,
                         BL_FLOAT threshold)
{
    if (mProcessNoise)
        return;
    
#define MIN_THRESHOLD -200.0
#define MAX_THRESHOLD 0.0

    // Optimization: precompute db
    WDL_TypedBuf<BL_FLOAT> &inputDB = mTmpBuf24;
    inputDB.Resize(width*height);
    BLUtils::AmpToDB(inputDB.Get(), input, inputDB.GetSize(),
                     (BL_FLOAT)BL_EPS, (BL_FLOAT)DENOISER_MIN_DB);
    BL_FLOAT *inputDBData = inputDB.Get();
    
    // Process only one line (for optimization)
    for (int j = lineNum; j < lineNum + 1; j++)
    {
        for (int i = 0; i < width; i++)
        {
            BL_FLOAT avg = 0.0;
            BL_FLOAT sum = 0.0;
            
            // By default, copy the input
            int index0 = i + j*width;
            
            output[index0] = input[index0];
            
            BL_FLOAT centerVal = input[index0];
            
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
                    BL_FLOAT val = inputDBData[x + y*width];
                    
                    BL_FLOAT kernelVal = 1.0;
                    if (kernel != NULL)
                    {
                        kernelVal = kernel->Get()[(wi + halfWinSize) + (wj + halfWinSize)*winSize];
                    }
                    
                    avg += val*kernelVal;
                    sum += kernelVal;
                }
            }
            
            if (sum > 0.0)
                avg /= sum;
            
            BL_FLOAT thrs = threshold*(MAX_THRESHOLD - MIN_THRESHOLD) + MIN_THRESHOLD;
            
            if (avg < thrs)
                output[index0] = 0.0;
        }
    }
}

void
DenoiserObj::SamplesHistoryToImage(const bl_queue<WDL_TypedBuf<BL_FLOAT> > *hist,
                                   WDL_TypedBuf<BL_FLOAT> *imageChunk)
{
    if (mProcessNoise)
        return;
    
    // Get the image dimensions
    int height = (int)hist->size();
    if (height < 1)
    {
        imageChunk->Resize(0);
        
        return;
    }
    
    const WDL_TypedBuf<BL_FLOAT> &hist0 = (*hist)[0];
    int width = hist0.GetSize();
    
    imageChunk->Resize(width*height);
    
    // Get the image buffer
    BL_FLOAT *imageBuf = imageChunk->Get();
    
    // Fill the image
    for (int j = 0; j < height; j++)
        // Time
    {
        const WDL_TypedBuf<BL_FLOAT> &histBuf = (*hist)[j];
        
        for (int i = 0; i < width; i++)
            // Bins
        {
            BL_FLOAT magn = histBuf.Get()[i];
            
            // Take appropriate scale
            BL_FLOAT logMagn = std::log(1.0 + magn);
            
            imageBuf[i + j*width] = logMagn;
        }
    }
}

void
DenoiserObj::ImageLineToSamples(const WDL_TypedBuf<BL_FLOAT> *image,
                                int width,
                                int height,
                                int lineNum,
                                const bl_queue<WDL_TypedBuf<BL_FLOAT> > *hist,
                                const bl_queue<WDL_TypedBuf<BL_FLOAT> > *phaseHist,
                                WDL_TypedBuf<BL_FLOAT> *resultBuf,
                                WDL_TypedBuf<BL_FLOAT> *resultPhases)
{
    if (mProcessNoise)
        return;
    
    if (lineNum >= height)
        return;
    
    if (lineNum >= resultBuf->GetSize())
        return;
    
    const WDL_TypedBuf<BL_FLOAT> &histLine = (*hist)[lineNum];
    const WDL_TypedBuf<BL_FLOAT> &phaseLine = (*phaseHist)[lineNum];
    
    *resultBuf = histLine;
    *resultPhases = phaseLine;
    
    // Process
    for (int i = 0; i < width; i++)
    {
        // Take the most recent
        BL_FLOAT logMagn = image->Get()[i + width*lineNum];
        
        BL_FLOAT newMagn = std::exp(logMagn) - 1.0;
        if (newMagn < 0.0)
            newMagn = 0.0;
        
        resultBuf->Get()[i] = newMagn;
    }
}

void
DenoiserObj::ExtractResidualNoise(const WDL_TypedBuf<BL_FLOAT> *prevSignal,
                                  const WDL_TypedBuf<BL_FLOAT> *signal,
                                  WDL_TypedBuf<BL_FLOAT> *ioNoise)
{
    if (mProcessNoise)
        return;
    
    for (int i = 0; i < ioNoise->GetSize(); i++)
    {
        BL_FLOAT prevMagn = prevSignal->Get()[i];
        BL_FLOAT magn = signal->Get()[i];
        BL_FLOAT noiseMagn = ioNoise->Get()[i];
        
        // Positive difference
        BL_FLOAT newMagn = noiseMagn + (prevMagn - magn);
        if (noiseMagn < 0.0)
            noiseMagn = 0.0;
        
        ioNoise->Get()[i] = newMagn;
    }
}

// See: http://home.mit.bme.hu/~bako/zaozeng/chapter4.htm
void
DenoiserObj::Threshold(WDL_TypedBuf<BL_FLOAT> *ioSigMagns,
                       WDL_TypedBuf<BL_FLOAT> *ioNoiseMagns)
{
    if (mProcessNoise)
        return;
    
    if (ioSigMagns->GetSize() != ioNoiseMagns->GetSize())
        return;
    
    WDL_TypedBuf<BL_FLOAT> &thrsNoiseMagns = mTmpBuf19;
    thrsNoiseMagns = *ioNoiseMagns;
    
    DenoiserObj::ApplyThresholdToNoiseCurve(&thrsNoiseMagns, mThreshold); // TODO DENOISER OPTIM this consumes a lot
    
    // Threshold, soft elbow
    for (int i = 0; i < ioSigMagns->GetSize(); i++)
    {
        BL_FLOAT magn = ioSigMagns->Get()[i];
        BL_FLOAT noise = thrsNoiseMagns.Get()[i];
        
        BL_FLOAT newMagn = (magn + 1.0)/(noise + 1.0) - 1.0;
        if (newMagn < 0.0)
            newMagn = 0.0;
        
        ioSigMagns->Get()[i] = newMagn;
        
        BL_FLOAT newNoise = magn - newMagn;
        if (newNoise < 0.0)
            newNoise = 0.0;
        
        ioNoiseMagns->Get()[i] = newNoise;
    }
}

void
DenoiserObj::ApplyThresholdToNoiseCurve(WDL_TypedBuf<BL_FLOAT> *ioNoiseCurve,
                                        BL_FLOAT threshold)
{
    // Apply threshold not in dB
    BL_FLOAT thrs0 = threshold*THRESHOLD_COEFF;
    for (int i = 0; i < ioNoiseCurve->GetSize(); i++)
    {
        BL_FLOAT sample = ioNoiseCurve->Get()[i];
        
        // NOTE: We don't have the problem of the flat curve at
        // the end of the graph with high sample rates
        
        sample *= thrs0;
        
        ioNoiseCurve->Get()[i] = sample;
    }
}

#if USE_VARIABLE_BUFFER_SIZE
void
DenoiserObj::ResampleNoisePattern()
{
    if (mProcessNoise)
        return;
    
    int bufferSize = BLUtilsPlug::PlugComputeBufferSize(DENOISER_BUFFER_SIZE,
                                                        mSampleRate);
    int currentNoisePatternSize = bufferSize/2;
    
    // Explanation: 1 fft bin has the same value for 44100Hz + buffer size 2048
    // as with 88200Hz + buffersize 4096 !
    // At 88200Hz, we can got to a frequency of 44100Hz at the maximum
    // (comparead to maximum 22050Hz at 44100Hz)
    mNoisePattern = mNativeNoisePattern;
    
    BLUtils::ResizeFillZeros(&mNoisePattern, currentNoisePatternSize);
}
#endif
