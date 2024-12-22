#ifndef WIENER_SOFT_MASKING_H
#define WIENER_SOFT_MASKING_H

#include <bl_queue.h>

#include <BLTypes.h>

#include "IPlug_include_in_plug_hdr.h"

#include "../../WDL/fft.h"

// Wiener soft masking applied on complexes
//
// See: https://github.com/TUIlmenauAMS/ASP/blob/master/MaskingMethods.py
// and: http://www.jonathanleroux.org/pdf/Erdogan2015ICASSP04.pdf
// and: https://www.researchgate.net/publication/220736985_Degenerate_Unmixing_Estimation_Technique_using_the_Constant_Q_Transform
// and: https://hal.inria.fr/inria-00544949/document
// and: https://hal.inria.fr/hal-01881425/document
class WienerSoftMasking
{
public:
    WienerSoftMasking(int bufferSize, int overlapping, int historySize);
    
    virtual ~WienerSoftMasking();

    void Reset();
    
    void Reset(int bufferSize, int overlapping);
    
    void SetHistorySize(int size);
    
    int GetHistorySize();
    
    void SetProcessingEnabled(bool flag);
    bool IsProcessingEnabled();

    int GetLatency();
    
    // Returns the centered data value in ioSum
    // Returns the centered masked data in ioMaskedResult0
    // Return sum - maskedResult0 in  ioMaskedResult1 if required
    void ProcessCentered(WDL_TypedBuf<WDL_FFT_COMPLEX> *ioSum,
                         const WDL_TypedBuf<BL_FLOAT> &mask,
                         WDL_TypedBuf<WDL_FFT_COMPLEX> *ioMaskedResult0,
                         WDL_TypedBuf<WDL_FFT_COMPLEX> *ioMaskedResult1 = NULL);
               
protected:
    void ComputeSigma2(WDL_TypedBuf<WDL_FFT_COMPLEX> *outSigma2Mask0,
                       WDL_TypedBuf<WDL_FFT_COMPLEX> *outSigma2Mask1);
                       
    //
    class HistoryLine
    {
    public:
        HistoryLine();
        HistoryLine(const HistoryLine &other);

        virtual ~HistoryLine();

        void Resize(int size);
        int GetSize();
        
    public:
        WDL_TypedBuf<WDL_FFT_COMPLEX> mSum;
        
        WDL_TypedBuf<WDL_FFT_COMPLEX> mMasked0Square;
        WDL_TypedBuf<WDL_FFT_COMPLEX> mMasked1Square;

    protected:
        int mSize;
    };

    //
    int mBufferSize;
    int mOverlapping;
    
    int mHistorySize;
    
    bl_queue<HistoryLine> mHistory;
    
    //
    WDL_TypedBuf<BL_FLOAT> mWindow;
    
    bool mProcessingEnabled;

private:
    HistoryLine mTmpHistoryLine;
    WDL_TypedBuf<WDL_FFT_COMPLEX> mTmpBuf0;
    WDL_TypedBuf<WDL_FFT_COMPLEX> mTmpBuf1;
    WDL_TypedBuf<WDL_FFT_COMPLEX> mTmpBuf2;
    WDL_TypedBuf<WDL_FFT_COMPLEX> mTmpBuf3;
    WDL_TypedBuf<WDL_FFT_COMPLEX> mTmpBuf4;
    WDL_TypedBuf<WDL_FFT_COMPLEX> mTmpBuf5;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf6;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf7;
};

#endif
