#include <Window.h>

#include <BLUtils.h>
#include <BLUtilsComp.h>
#include <BLUtilsMath.h>

#include "WienerSoftMasking.h"

// Optimization
#define USE_FAKE_MASK1 1

WienerSoftMasking::HistoryLine::HistoryLine()
{
    mSize = 0;
}

WienerSoftMasking::HistoryLine::HistoryLine(const HistoryLine &other)
{
    mSize = other.mSize;

    Resize(mSize);
    
    mSum = other.mSum;
    mMasked0Square = other.mMasked0Square;
    mMasked1Square = other.mMasked1Square;
}

WienerSoftMasking::HistoryLine::~HistoryLine() {}

void
WienerSoftMasking::HistoryLine::Resize(int size)
{
    mSize = size;
    
    mSum.Resize(mSize);
    mMasked0Square.Resize(mSize);
    mMasked1Square.Resize(mSize);
}

int
WienerSoftMasking::HistoryLine::GetSize()
{
    return mSize;
}

//

WienerSoftMasking::WienerSoftMasking(int bufferSize, int overlapping, int historySize)
{
    mBufferSize = bufferSize;
    mOverlapping = overlapping;
    
    mHistorySize = historySize;
    
    mProcessingEnabled = true;
}

WienerSoftMasking::~WienerSoftMasking() {}

void
WienerSoftMasking::Reset(int bufferSize, int overlapping)
{
    mBufferSize = bufferSize;
    mOverlapping = overlapping;
    
    Reset();
}

void
WienerSoftMasking::Reset()
{    
    mHistory.resize(0);
}

void
WienerSoftMasking::SetHistorySize(int size)
{
    mHistorySize = size;
    
    Reset();
}

int
WienerSoftMasking::GetHistorySize()
{
    return mHistorySize;
}

void
WienerSoftMasking::SetProcessingEnabled(bool flag)
{
    mProcessingEnabled = flag;
}

bool
WienerSoftMasking::IsProcessingEnabled()
{
    return mProcessingEnabled;
}

int
WienerSoftMasking::GetLatency()
{
#if 0 // Version with hack
    int latency = (mHistorySize/2)*(mBufferSize/mOverlapping);
    // Hack (for Air)
    latency *= 0.75;
#endif

    // Correct version
    //
    // In history, we push_back() and pop_front()
    // and we take the index historySize/2
    //

    // The index where we get the data (from the end)
    // (this covers the case of odd and even history size)
    // Index 0 has 0 latency, since we have just added the current data to it.
    int revIndex = (mHistorySize - 1) - mHistorySize/2;
    int latency = revIndex*(mBufferSize/mOverlapping);
    
    return latency;
}

// Process over time
//
// Algo:

// s = input * HM
// n = input * (1.0 - HM)
// SM = s2(s)/(s2(s) + s2(n))
// output = input * SM
//
void
WienerSoftMasking::ProcessCentered(WDL_TypedBuf<WDL_FFT_COMPLEX> *ioSum,
                                  const WDL_TypedBuf<BL_FLOAT> &mask,
                                  WDL_TypedBuf<WDL_FFT_COMPLEX> *ioMaskedResult0,
                                  WDL_TypedBuf<WDL_FFT_COMPLEX> *ioMaskedResult1)
{

    HistoryLine &newHistoLine = mTmpHistoryLine;
    newHistoLine.Resize(ioSum->GetSize());

    newHistoLine.mSum = *ioSum;

    // Optim: compute square history only if enabled
    // Otherwise, fill with zeros
    if (mProcessingEnabled)
    {
        // masked0 = sum*mask
        newHistoLine.mMasked0Square = *ioSum;
        BLUtils::MultValues(&newHistoLine.mMasked0Square, mask);

        // maskd1 = sum - masked0
        // same as: masked1 = sum*(1 - mask)
        newHistoLine.mMasked1Square = *ioSum;
        BLUtils::SubstractValues(&newHistoLine.mMasked1Square,
                                 newHistoLine.mMasked0Square);
        
        // See: https://hal.inria.fr/hal-01881425/document
        // |x|^2
        // NOTE: square abs => complex conjugate
        
        // Compute squares (using complex conjugate)
        BLUtilsComp::ComputeSquareConjugate(&newHistoLine.mMasked0Square);
        BLUtilsComp::ComputeSquareConjugate(&newHistoLine.mMasked1Square);
    }
    else // Not enabled, fill history with zeros
    {
        newHistoLine.mMasked0Square.Resize(ioSum->GetSize());
        BLUtils::FillAllZero(&newHistoLine.mMasked0Square);
        
        newHistoLine.mMasked1Square.Resize(ioSum->GetSize());
        BLUtils::FillAllZero(&newHistoLine.mMasked1Square);
    }
    
    // Manage the history
    if (mHistory.empty())
    {
        // Fill the whole history with the first line
        mHistory.resize(mHistorySize);
        mHistory.clear(newHistoLine);
    }
    else
    {
        mHistory.freeze();
        mHistory.push_pop(newHistoLine);
    }
    
    if (mProcessingEnabled)
    {
        WDL_TypedBuf<WDL_FFT_COMPLEX> &sigma2Mask0 = mTmpBuf0;
        WDL_TypedBuf<WDL_FFT_COMPLEX> &sigma2Mask1 = mTmpBuf1;
        ComputeSigma2(&sigma2Mask0, &sigma2Mask1);

        WDL_FFT_COMPLEX *s0Data = sigma2Mask0.Get();
        WDL_FFT_COMPLEX *s1Data = sigma2Mask1.Get();
        
        // Create the mask
        if (mHistory.empty()) // Just in case
            return;

        WDL_TypedBuf<WDL_FFT_COMPLEX> &softMask0 = mTmpBuf4;
        softMask0.Resize(mHistory[0].GetSize());

        int softMask0Size = softMask0.GetSize();
        WDL_FFT_COMPLEX *softMask0Data = softMask0.Get();
        
        // Compute soft mask 0
        WDL_FFT_COMPLEX csum;
        WDL_FFT_COMPLEX maskVal;
        for (int i = 0; i < softMask0Size; i++)
        {
            const WDL_FFT_COMPLEX &s0 = s0Data[i];
            const WDL_FFT_COMPLEX &s1 = s1Data[i];

            // Compute s0 + s1
            csum = s0;
            csum.re += s1.re;
            csum.im += s1.im;

            maskVal.re = 0.0;
            maskVal.im = 0.0;

            if ((std::fabs(csum.re) > BL_EPS) ||
                (std::fabs(csum.im) > BL_EPS))
            {
                COMP_DIV(s0, csum, maskVal);
            }
            
            BL_FLOAT maskMagn = COMP_MAGN(maskVal);

            // Limit to 1
            if (maskMagn >  1.0)
            {
                BL_FLOAT maskMagnInv = 1.0/maskMagn;
                maskVal.re *= maskMagnInv;
                maskVal.im *= maskMagnInv;
            }
            
            softMask0Data[i] = maskVal;
        }

        // Result when enabled
        
        // Apply mask 0
        *ioMaskedResult0 = mHistory[mHistory.size()/2].mSum;
        BLUtils::MultValues(ioMaskedResult0, softMask0);

        // Mask 1
        if (ioMaskedResult1 != NULL)
        {
#if USE_FAKE_MASK1
            // Fake mask 1
            // Simple difference
            *ioMaskedResult1 = mHistory[mHistory.size()/2].mSum;;
            BLUtils::SubstractValues(ioMaskedResult1, *ioMaskedResult0);
#else
            // Use real mask for second mask
            
            WDL_TypedBuf<WDL_FFT_COMPLEX> &softMask1 = mTmpBuf5;
            softMask1.Resize(mHistory[0].GetSize());

            int softMask1Size = softMask1.GetSize();
            WDL_FFT_COMPLEX *softMask1Data = softMask1.Get();
        
            // Compute soft mask 1
            WDL_FFT_COMPLEX csum;
            WDL_FFT_COMPLEX maskVal;
            for (int i = 0; i < softMask1Size; i++)
            {
                // TODO: optimize, by re-using softMask0
                // and making 1 - mask0, with complex
                
                const WDL_FFT_COMPLEX &s0 = s0Data[i];
                const WDL_FFT_COMPLEX &s1 = s1Data[i];
                
                // Compute s0 + s1
                csum = s0;
                csum.re += s1.re;
                csum.im += s1.im;
        
                if ((std::fabs(csum.re) > BL_EPS) ||
                    (std::fabs(csum.im) > BL_EPS))
                {
                    COMP_DIV(s1, csum, maskVal);
                }

                BL_FLOAT maskMagn = COMP_MAGN(maskVal);

                // Limit to 1
                if (maskMagn >  1.0)
                {
                    BL_FLOAT maskMagnInv = 1.0/maskMagn;
                    maskVal.re *= maskMagnInv;
                    maskVal.im *= maskMagnInv;
                }
                    
                softMask1Data[i] = maskVal;
            }

            // Apply mask 1
            *ioMaskedResult1 = mHistory[mHistory.size()/2].mSum;
            BLUtils::MultValues(ioMaskedResult1, softMask1);
#endif
        }
    }
    
    // Even if processing enabled is true or false,
    // update the data from the history
    
    // Compute the centered values
    if (!mHistory.empty())
    {
        // Shifted input data
        *ioSum = mHistory[mHistory.size()/2].mSum;
    }
}

// NOTE: variance is equal to sigma^2
void
WienerSoftMasking::ComputeSigma2(WDL_TypedBuf<WDL_FFT_COMPLEX> *outSigma2Mask0,
                                 WDL_TypedBuf<WDL_FFT_COMPLEX> *outSigma2Mask1)
{    
    if (mHistory.empty())
        return;
    
    outSigma2Mask0->Resize(mHistory[0].GetSize());
    outSigma2Mask1->Resize(mHistory[0].GetSize());
    
    // Result sum 0
    WDL_TypedBuf<WDL_FFT_COMPLEX> &currentSum0 = mTmpBuf2;
    currentSum0.Resize(mHistory[0].GetSize());
    BLUtils::FillAllZero(&currentSum0);

    WDL_FFT_COMPLEX *currentSum0Data = currentSum0.Get();
    
    // Result sum 1
    WDL_TypedBuf<WDL_FFT_COMPLEX> &currentSum1 = mTmpBuf3;
    currentSum1.Resize(mHistory[0].GetSize());
    BLUtils::FillAllZero(&currentSum1);

    WDL_FFT_COMPLEX *currentSum1Data = currentSum1.Get();
    
    // Window
    if (mWindow.GetSize() != mHistory.size())
    {
        Window::MakeHanning(mHistory.size(), &mWindow);
    }
    
    BL_FLOAT sumProba = BLUtils::ComputeSum(mWindow);
    BL_FLOAT sumProbaInv = 0.0;
    if (sumProba > BL_EPS)
        sumProbaInv = 1.0/sumProba;
    
    //
    for (int j = 0; j < mHistory.size(); j++)
    {
        const HistoryLine &line = mHistory[j];
        
        const WDL_TypedBuf<WDL_FFT_COMPLEX> &line0 = line.mMasked0Square;
        int line0Size = line0.GetSize();
        WDL_FFT_COMPLEX *line0Data = line0.Get();
        
        const WDL_TypedBuf<WDL_FFT_COMPLEX> &line1 = line.mMasked1Square;
        WDL_FFT_COMPLEX *line1Data = line1.Get();
        
        BL_FLOAT p = mWindow.Get()[j];
        
        for (int i = 0; i < line0Size; i++)
        {
            WDL_FFT_COMPLEX &expect0 = currentSum0Data[i];
            const WDL_FFT_COMPLEX &val0 = line0Data[i];
            expect0.re += p*val0.re;
            expect0.im += p*val0.im;

            WDL_FFT_COMPLEX &expect1 = currentSum1Data[i];
            const WDL_FFT_COMPLEX &val1 = line1Data[i];
            expect1.re += p*val1.re;
            expect1.im += p*val1.im;
        }
    }

    // Divide by sum probas
    if (sumProba > BL_EPS)
    {
        BLUtils::MultValues(&currentSum0, sumProbaInv);
        BLUtils::MultValues(&currentSum1, sumProbaInv);
    }
    
    // Result
    *outSigma2Mask0 = currentSum0;
    *outSigma2Mask1 = currentSum1;
}
