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

#ifndef PARTIAL_TRACKER_H
#define PARTIAL_TRACKER_H

#include <vector>
using namespace std;

#include <Scale.h>

#include <PeakDetector.h>

#include <Partial.h>

#include "IPlug_include_in_plug_hdr.h"

#include "../../WDL/fft.h"

// Use PeakDetector class, original BlueLab implementation
#define USE_BL_PEAK_DETECTOR 0 //1

// Use smart peak detection from http://billauer.co.il/peakdet.html
// See also: https://github.com/xuphys/peakdetect/blob/master/peakdetect.c
//
// NOTE: this must be define in header, because some GUI controls need it
#define USE_BILLAUER_PEAK_DETECTOR 1 // 0

#define USE_PARTIAL_FILTER_MARCHAND 0 //1
#define USE_PARTIAL_FILTER_AMFM 1 //0

// Detect partials
//
#if USE_BL_PEAK_DETECTOR
#define DETECT_PARTIALS_START_INDEX 2
#else
// ORIGIN
//#define DETECT_PARTIALS_START_INDEX 1
// Better, in order to not discard low freq peaks with Billauer real prominence
#define DETECT_PARTIALS_START_INDEX 0
#endif

// PartialTracker2
// - from PartialTracker
//
// For association, sort by biggest amplitudes,
// and use a freqency threshold instead of taking the nearest
//
// Compute predicted next ppartial and associate using prediction
//
//
// PartialTracker3:
// - amps in Db, and magns scaled to Mel
//
// PartialTracker4:
// - fix partial amp computation
// (was sometimes really under the correct value for Infra, tested on
// "Bass-065_keep-the-bass-dubby"

// PartialTracker5: improvement of trakcng quality, for new SASViewer
// NOTE: previously test FreqAdjustObj. this was not efficient (wobbling)
//class FreqAdjustObj3;
class PartialFilter;
class PartialTracker
{
public:
    PartialTracker7(int bufferSize, BL_FLOAT sampleRate,
                    BL_FLOAT overlapping);
    
    virtual ~PartialTracker7();
    
    void Reset();
    
    void Reset(int bufferSize, BL_FLOAT sampleRate);
    
    BL_FLOAT GetMinAmpDB();
    
    void SetThreshold(BL_FLOAT threshold);
    void SetThreshold2(BL_FLOAT threshold2);

    // Use one or the other following methods
    //
    
    // Magn/phase
    void SetData(const WDL_TypedBuf<BL_FLOAT> &magns,
                 const WDL_TypedBuf<BL_FLOAT> &phases);

    // Complex
    void SetData(const WDL_TypedBuf<WDL_FFT_COMPLEX> &data);
    
    void GetPreProcessedMagns(WDL_TypedBuf<BL_FLOAT> *magns);
    
    //
    void DetectPartials();
    void FilterPartials();
    
    void GetPartials(vector<Partial> *partials);

    // For getting current partials before filtering
    void GetRawPartials(vector<Partial> *partials);
    
    void ClearResult();
    
    // Maximum frequency we try to detect (limit for BL-Infra for example)
    void SetMaxDetectFreq(BL_FLOAT maxFreq);
    
    // Preprocess time smooth
    void SetTimeSmoothCoeff(BL_FLOAT coeff);
    
    // For processing result warping for example
    void PreProcessDataX(WDL_TypedBuf<BL_FLOAT> *data);

    //
    void PreProcessDataY(WDL_TypedBuf<BL_FLOAT> *data);
    
    // For processing result color for example, just before display
    void PreProcessDataXY(WDL_TypedBuf<BL_FLOAT> *data);
    
    // Unwrap phases for interpolatin in Mel
    void PreProcessUnwrapPhases(WDL_TypedBuf<BL_FLOAT> *magns,
                                WDL_TypedBuf<BL_FLOAT> *phases);
    
    void DenormPartials(vector<Partial> *partials);
    void DenormData(WDL_TypedBuf<BL_FLOAT> *data);
    
    void PartialsAmpToAmpDB(vector<Partial> *partials);

    BL_FLOAT PartialScaleToQIFFTScale(BL_FLOAT ampDbNorm);
    BL_FLOAT QIFFTScaleToPartialScale(BL_FLOAT ampLog);

    void SetNeriDelta(BL_FLOAT delta);
    
protected:
    // Pre process
    //
    void PreProcess(WDL_TypedBuf<BL_FLOAT> *magns,
                    WDL_TypedBuf<BL_FLOAT> *phases);
    
    // Do it in the complex domain (to be compatible with AM/FM parameters)
    void PreProcessTimeSmooth(WDL_TypedBuf<WDL_FFT_COMPLEX> *data);
    
    // Apply A-Weighting, so the peaks at highest frequencies will not be small
    void PreProcessAWeighting(WDL_TypedBuf<BL_FLOAT> *magns, bool reverse = false);
    
    BL_FLOAT ProcessAWeighting(int binNum, int numBins,
                               BL_FLOAT magn, bool reverse);


    void ComputePartials(const vector<PeakDetector::Peak> &peaks,
                         const WDL_TypedBuf<BL_FLOAT> &magns,
                         const WDL_TypedBuf<BL_FLOAT> &phases,
                         vector<Partial> *partials);

    void RemoveRealDeadPartials(vector<Partial> *partials);
        
    // Detection
    //
    void DetectPartials(const WDL_TypedBuf<BL_FLOAT> &magns,
                        const WDL_TypedBuf<BL_FLOAT> &phases,
                        vector<Partial> *partials);
    
    // Peak frequency computation
    //
    BL_FLOAT ComputePeakIndexAvg(const WDL_TypedBuf<BL_FLOAT> &magns,
                                 int leftIndex, int rightIndex);
    
    BL_FLOAT ComputePeakIndexParabola(const WDL_TypedBuf<BL_FLOAT> &magns,
                                      int peakIndex);
    
    // Peak amp
    //
    BL_FLOAT ComputePeakAmpInterp(const WDL_TypedBuf<BL_FLOAT> &magns,
                                  BL_FLOAT peakFreq);
    
    void ComputePeakMagnPhaseInterp(const WDL_TypedBuf<BL_FLOAT> &magns,
                                    const WDL_TypedBuf<BL_FLOAT> &unwrappedPhases,
                                    BL_FLOAT peakFreq,
                                    BL_FLOAT *peakAmp, BL_FLOAT *peakPhase);
    
    
    // Glue the barbs to the main partial
    // Return true if some barbs have been glued
    bool GluePartialBarbs(const WDL_TypedBuf<BL_FLOAT> &magns,
                          vector<Partial> *partials);
     
    // Discard partials which are almost flat
    // (compare height of the partial, and width in the middle
    bool DiscardFlatPartial(const WDL_TypedBuf<BL_FLOAT> &magns,
                            int peakIndex, int leftIndex, int rightIndex);
    
    void DiscardFlatPartials(const WDL_TypedBuf<BL_FLOAT> &magns,
                             vector<Partial> *partials);
    
    // Suppress partials with zero frequencies
    void SuppressZeroFreqPartials(vector<Partial> *partials);

    BL_FLOAT ComputePeakHeight(const WDL_TypedBuf<BL_FLOAT> &magns,
                               int peakIndex, int leftIndex, int rightIndex);
    void ThresholdPartialsPeakHeight(const WDL_TypedBuf<BL_FLOAT> &magns,
                                     vector<Partial> *partials);
    
    // Peaks
    //
    
    // Fixed
    BL_FLOAT ComputePeakProminence(const WDL_TypedBuf<BL_FLOAT> &magns,
                                   int peakIndex, int leftIndex, int rightIndex);

    BL_FLOAT ComputePeakHigherFoot(const WDL_TypedBuf<BL_FLOAT> &magns,
                                 int leftIndex, int rightIndex);

    BL_FLOAT ComputePeakLowerFoot(const WDL_TypedBuf<BL_FLOAT> &magns,
                                  int leftIndex, int rightIndex);
  
    //    
    
    // Adaptive threshold, depending on bin num;
    BL_FLOAT GetThreshold(int binNum);

    // Optim: pre-compute a weights
    void ComputeAWeights(int numBins, BL_FLOAT sampleRate);
        
    // Debug
    // First method
    void DBG_DumpPartials(const char *fileName,
                          const vector<Partial> &partials,
                          int bufferSize);

    int DenormBinIndex(int idx);

    void PostProcessPartials(const WDL_TypedBuf<BL_FLOAT> &magns,
                             vector<Partial> *partials);

    void DBG_DumpPeaks(const WDL_TypedBuf<BL_FLOAT> &data,
                       const vector<PeakDetector::Peak> &peaks);

    // Second method
    void DBG_DumpPartials(const WDL_TypedBuf<BL_FLOAT> &magns,
                          const vector<Partial> &partials);
        
    //
    int mBufferSize;
    BL_FLOAT mSampleRate;
    int mOverlapping;
    
    BL_FLOAT mThreshold;
    
    //
    WDL_TypedBuf<BL_FLOAT> mCurrentMagns;
    WDL_TypedBuf<BL_FLOAT> mCurrentPhases;

    WDL_TypedBuf<BL_FLOAT> mLinearMagns;
    WDL_TypedBuf<BL_FLOAT> mLogMagns;
    
    vector<Partial> mResult;
    
    //
    BL_FLOAT mMaxDetectFreq;
    
    // For Pre-Process
    BL_FLOAT mTimeSmoothCoeff;
    // Smooth only magns (tried smooth complex, but that was bad)
    WDL_TypedBuf<BL_FLOAT> mPrevMagns;
    // When using complexex...
    WDL_TypedBuf<WDL_FFT_COMPLEX> mPrevComp;
    
    // Scales
    Scale *mScale;
    Scale::Type mXScale;
    Scale::Type mYScale;
    Scale::Type mYScale2; // For log
    
    Scale::Type mXScaleInv;
    Scale::Type mYScaleInv;
    Scale::Type mYScaleInv2; // for log

    // Optim
    WDL_TypedBuf<BL_FLOAT> mAWeights;

    PeakDetector *mPeakDetector;
    PartialFilter *mPartialFilter;
    
private:
    // Tmp buffers
    WDL_TypedBuf<BL_FLOAT> mTmpBuf0;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf1;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf2;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf3;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf4;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf5;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf6;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf7;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf8;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf9;
    WDL_TypedBuf<BL_FLOAT> mTmpBuf10;
    WDL_TypedBuf<WDL_FFT_COMPLEX> mTmpBuf11;
    
    vector<Partial> mTmpPartials0;
    vector<Partial> mTmpPartials1;
    vector<Partial> mTmpPartials2;
    vector<Partial> mTmpPartials3;
    vector<Partial> mTmpPartials4;
    vector<Partial> mTmpPartials5;
    vector<Partial> mTmpPartials6;
    vector<Partial> mTmpPartials7;
    vector<Partial> mTmpPartials8;
    vector<Partial> mTmpPartials9;
    vector<Partial> mTmpPartials10;
    vector<Partial> mTmpPartials11;
};

#endif
