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

#include <BLUtils.h>
#include <BLUtilsMath.h>

#include <BLDebug.h>

#include "PeakDetectorBillauer.h"

// See also: Matlab peak detection (ofr prominence and this kind of stuff
// https://fr.mathworks.com/help/signal/ref/findpeaks.html

// Keep 20 peaks and suppress only if more than 20 peaks 
#define SUPPRESS_MIN_NUM_PEAKS 20.0

PeakDetectorBillauer::PeakDetectorBillauer(BL_FLOAT maxDelta)
{    
    mMaxDelta = maxDelta;

    //BL_FLOAT threshold = 0.25;
    BL_FLOAT threshold = 0.01; // Morpho
    
    mDelta = threshold*mMaxDelta;

    mThreshold2 = 1.0;
}

PeakDetectorBillauer::~PeakDetectorBillauer() {}

void
PeakDetectorBillauer::SetThreshold(BL_FLOAT threshold)
{   
    mDelta = threshold*mMaxDelta;
}

void
PeakDetectorBillauer::SetThreshold2(BL_FLOAT threshold2)
{
    mThreshold2 = threshold2;
}

void
PeakDetectorBillauer::
DetectPeaks(const WDL_TypedBuf<BL_FLOAT> &data, vector<Peak> *peaks,
            int minIndex, int maxIndex)
{
    // Arguments
    if (minIndex < 0)
        minIndex = 0;
    if (maxIndex < 0)
        maxIndex = data.GetSize() - 1;
    
    // First, fill the min and max arrays
    //
    vector<int> mintab;
    vector<int> maxtab;

    BL_FLOAT mn = BL_INF;
    BL_FLOAT mx = -BL_INF;
    int mnpos = -1;
    int mxpos = -1;

    // Look for max first
    bool lookformax = true;

    // Check if we start by a peak
    bool startedbypeak = false;
    if (maxIndex - minIndex >= 2)
    {
        BL_FLOAT val0 = data.Get()[minIndex];
        BL_FLOAT val1 = data.Get()[minIndex + 1];

        if (val0 > val1)
            // We are starting on a descending slope
        {
            // Start by looking for min
            maxtab.push_back(minIndex);
            mx = val0;
            mxpos = minIndex;
            lookformax = false;

            startedbypeak = true;
        }
    }
    
    for (int i = minIndex; i <= maxIndex; i++)
    {
        BL_FLOAT t = data.Get()[i];
        
        if (t > mx)
        {
            mx = t;
            mxpos = i;
        }
        
        if (t < mn)
        {
            mn = t;
            mnpos = i;
        }

        if (lookformax)
        {
            if (t < mx - mDelta)
            {
                maxtab.push_back(mxpos);
                mn = t;
                mnpos = i;
                lookformax = false;

                // See: https://github.com/xuphys/peakdetect/blob/master/peakdetect.c
                //i = mxpos - 1;
            }
        }
        else
        {
            if (t > mn + mDelta)
            {
                mintab.push_back(mnpos);
                mx = t;
                mxpos = i;
                lookformax = true;

                // See: https://github.com/xuphys/peakdetect/blob/master/peakdetect.c
                //i = mnpos - 1;
            }
        }
    }

    // We started by a peak
    bool keepfirstpeak = true;
    if (startedbypeak)
    {
        // Check if we must keep the first peak
        if ((maxtab.size() > 0) && (mintab.size() > 0))
        {
            if (!(data.Get()[maxtab[0]] >= data.Get()[mintab[0]] + mDelta))
                keepfirstpeak = false;
        }
    }
    
    // Secondly, fill the peaks
    //
    //peaks->resize(maxtab.size());
    peaks->clear();
    for (int i = 0; i < maxtab.size(); i++)
    {
        if ((i == 0) && !keepfirstpeak)
            continue;
        
        Peak peak;
        peak.mPeakIndex = maxtab[i];
        peak.mLeftIndex = ((i - 1 >= 0) && (i - 1 < mintab.size())) ?
            mintab[i - 1] : minIndex;
        peak.mRightIndex = (i < mintab.size()) ? mintab[i] : maxIndex;
        
        peaks->push_back(peak);
    }
    
    // Post process
    //
    
    //SuppressSmallPeaks(data, peaks);

    // Simple, does not enlarge too much
    AdjustPeaksWidthSimple(data, peaks, minIndex, maxIndex);
    
    // NOTE: this may not narrow very small peaks surrounded by noise
    //AdjustPeaksWidthProminence(data, peaks, minIndex, maxIndex);
    
    //SuppressSmallPeaksProminence(data, peaks, minIndex, maxIndex);
    SuppressSmallPeaksFrequency(data, peaks, minIndex, maxIndex);
}

// Simple method, with hard coded value
void
PeakDetectorBillauer::SuppressSmallPeaksSimple(const WDL_TypedBuf<BL_FLOAT> &data,
                                               vector<Peak> *peaks)
{
    // Peaks must be +2dB over the noise floor
    // => this will avoid having many many flat partials at high frequencies
#define MIN_PARTIAL_HEIGHT_OVER_NF 0.0 //0.5 //2.0
    
    vector<Peak> newPeaks;
    for (int i = 0; i < peaks->size(); i++)
    {
        const Peak &peak = (*peaks)[i];
        
        BL_FLOAT lm = data.Get()[peak.mLeftIndex];
        BL_FLOAT rm = data.Get()[peak.mRightIndex];

        BL_FLOAT base = (lm < rm) ? lm : rm;

        BL_FLOAT height = data.Get()[peak.mPeakIndex] - base;

        if (height >= MIN_PARTIAL_HEIGHT_OVER_NF)
            newPeaks.push_back(peak);
    }
    
    *peaks = newPeaks;
}

void
PeakDetectorBillauer::SuppressSmallPeaksProminence(const WDL_TypedBuf<BL_FLOAT> &data,
                                                   vector<Peak> *peaks,
                                                   int minIndex, int maxIndex)
{
    if (mThreshold2 >= 1.0)
        // Take all peaks
        return;

    if (peaks->size() < SUPPRESS_MIN_NUM_PEAKS)
        return;

    // Use real prominence (matlab algo)
    //
    // (If using the simple prominence method, we would suppress some segments
    // of really big peaks) 
    ComputePeaksProminence(data, peaks, minIndex, maxIndex);

    // Sort peaks by prominence
    sort(peaks->begin(), peaks->end(), Peak::ProminenceLess);

    // Order biggest peaks first
    reverse(peaks->begin(), peaks->end());

    // Keep only the biggest peaks
    int numToTakePeaks = peaks->size()*mThreshold2;

    if ((numToTakePeaks < SUPPRESS_MIN_NUM_PEAKS) &&
        (peaks->size() > SUPPRESS_MIN_NUM_PEAKS))
        numToTakePeaks = SUPPRESS_MIN_NUM_PEAKS;
    
    peaks->resize(numToTakePeaks);
}

void
PeakDetectorBillauer::SuppressSmallPeaksFrequency(const WDL_TypedBuf<BL_FLOAT> &data,
                                                  vector<Peak> *peaks,
                                                  int minIndex, int maxIndex)
{
    if (mThreshold2 >= 1.0)
        // Take all peaks
        return;

    if (peaks->size() < SUPPRESS_MIN_NUM_PEAKS)
        return;

    // Sort peaks by prominence
    sort(peaks->begin(), peaks->end(), Peak::PeakIndexLess);

    // Keep only the biggest peaks
    int numToTakePeaks = peaks->size()*mThreshold2;

    if ((numToTakePeaks < SUPPRESS_MIN_NUM_PEAKS) &&
        (peaks->size() > SUPPRESS_MIN_NUM_PEAKS))
        numToTakePeaks = SUPPRESS_MIN_NUM_PEAKS;
    
    peaks->resize(numToTakePeaks);
}

// With Billauer, and only one peak, left and right peak indices will
// be near to the minimum and maximum
//
// => So recompute the peak bounds, so they really match the peak,
// and they not cover the whole range of frequencies
//
// NOTE: this is good!
// Remaining problem: high harmonics + noisy signal => peaks are too large
void
PeakDetectorBillauer::AdjustPeaksWidthSimple(const WDL_TypedBuf<BL_FLOAT> &data,
                                             vector<Peak> *peaks,
                                             int minIndex, int maxIndex)
{    
    for (int i = 0; i < peaks->size(); i++)
    {
        Peak &peak = (*peaks)[i];

        BL_FLOAT peakAmp = data.Get()[peak.mPeakIndex];

        // If too low, some harmonic parts will go to the noise envelope
        //#define PEAKS_WIDTH_RATIO 0.5 //0.2
        
        // First version: simple, bug in db
        //BL_FLOAT thrs = peakAmp*PEAKS_WIDTH_RATIO;
        
        // Second version: simple, manages db well
        //Also manage dB if it is the case
        //BL_FLOAT thrs = peakAmp - std::fabs(peakAmp*PEAKS_WIDTH_RATIO);

        // Third version: use prominence => better! 
#define PEAKS_WIDTH_RATIO2 0.75 
        ComputePeakProminenceSimple(data, &peak);
        BL_FLOAT thrs = peakAmp - std::fabs(peak.mProminence*PEAKS_WIDTH_RATIO2);
        
        // Make sure not to enlarge more than prev Billauer minima
        int originLeftIndex = peak.mLeftIndex;
        int originRightIndex = peak.mRightIndex;
        
        // Adjust left index
        for (int j = peak.mPeakIndex - 1; j >= minIndex; j--)
        {
            if (j <= originLeftIndex)
                break;
            
            BL_FLOAT a = data.Get()[j];
            
            // Test decrease
            if (a < thrs)
            {
                peak.mLeftIndex = j;
                
                break;
            }
        }

        // Adjust right index
        for (int j = peak.mPeakIndex + 1; j <= maxIndex; j++)
        {
            if (j >= originRightIndex)
                break;
            
            BL_FLOAT a = data.Get()[j];

            // Test decrease
            if (a < thrs)
            {
                peak.mRightIndex = j;
                
                break;
            }
        }

        // Avoid very asymetrical peaks
        // => make all peaks symetric, using minimal width distance
        int leftWidth = peak.mPeakIndex - peak.mLeftIndex;
        int rightWidth = peak.mRightIndex - peak.mPeakIndex;
        if (leftWidth > rightWidth)
            peak.mLeftIndex = peak.mPeakIndex - rightWidth;
        else if (rightWidth > leftWidth)
            peak.mRightIndex = peak.mPeakIndex + leftWidth;
    }
}

// With Billauer, minima can drift very far from the peak if the curve
// after the peak is flat, or with flat noise
//
// Use prominence to narrow the peak width if necessary
void
PeakDetectorBillauer::AdjustPeaksWidthProminence(const WDL_TypedBuf<BL_FLOAT> &data,
                                                 vector<Peak> *peaks,
                                                 int minIndex, int maxIndex)
{    
    for (int i = 0; i < peaks->size(); i++)
    {
        Peak &peak = (*peaks)[i];

        // Use simple promince (take the max of the surrounding valleys)
        // => better like this!
        ComputePeakProminenceSimple(data, &peak);

        BL_FLOAT peakData = data.Get()[peak.mPeakIndex];

        for (int j = peak.mPeakIndex - 1; j >= peak.mLeftIndex; j--)
        {
            BL_FLOAT val = data.Get()[j];
            BL_FLOAT prominence = peakData - val;
            
            if (prominence > peak.mProminence)
            {
                if (j + 1 < peak.mPeakIndex)
                    peak.mLeftIndex = j + 1;

                break;
            }
        }

        for (int j = peak.mPeakIndex + 1; j < peak.mRightIndex; j++)
        {
            BL_FLOAT val = data.Get()[j];
            BL_FLOAT prominence = peakData - val;
            
            if (prominence > peak.mProminence)
            {
                if (j - 1 > peak.mPeakIndex)
                    peak.mRightIndex = j - 1;
                
                break;
            }
        }
    }
}

// Prominence is computed from the highest border
//
// See: https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.peak_prominences.html#scipy.signal.peak_prominences
void
PeakDetectorBillauer::ComputePeakProminenceSimple(const WDL_TypedBuf<BL_FLOAT> &data,
                                                  Peak *peak)
{
    BL_FLOAT lm = data.Get()[peak->mLeftIndex];
    BL_FLOAT rm = data.Get()[peak->mRightIndex];

    BL_FLOAT base = (lm > rm) ? lm : rm;

    peak->mProminence = data.Get()[peak->mPeakIndex] - base;
}

// Real prominence. 
// See algorithm here:
// https://fr.mathworks.com/help/signal/ug/prominence.html#d123e28940
void
PeakDetectorBillauer::ComputePeakProminence(const WDL_TypedBuf<BL_FLOAT> &data,
                                            Peak *peak, int minIndex, int maxIndex)
{
    // Extend a horizontal line from the peak to the left and right until
    // the line does one of the following:
    // - Crosses the signal because there is a higher peak
    // - Reaches the left or right end of the signal
    
    BL_FLOAT peakVal = data.Get()[peak->mPeakIndex];
        
    int leftIndex = peak->mPeakIndex;
    int rightIndex = peak->mPeakIndex;
    
    for (int j = peak->mPeakIndex; j >= minIndex; j--)
    {
        BL_FLOAT val = data.Get()[j];
        if (val > peakVal)
            break;
        
        leftIndex = j;
    }
    
    for (int j = peak->mPeakIndex; j <= maxIndex; j++)
    {
        BL_FLOAT val = data.Get()[j];
        if (val > peakVal)
            break;
        
        rightIndex = j;
    }
    
    //Find the minimum of the signal in each of the two intervals defined
    // in Step 2. This point is either a valley or one of the signal endpoints.
    BL_FLOAT leftMin = peakVal;
    BL_FLOAT rightMin = peakVal;
    
    for (int j = peak->mPeakIndex; j >= leftIndex; j--)
    {
        BL_FLOAT val = data.Get()[j];
        
        if (val < leftMin)
            leftMin = val;
    }
    
    for (int j = peak->mPeakIndex; j <= rightIndex; j++)
    {
        BL_FLOAT val = data.Get()[j];
        
        if (val < rightMin)
            rightMin = val;
    }
    
    // The higher of the two interval minima specifies the reference level.
    // The height of the peak above this level is its prominence.
    
    BL_FLOAT prominence = (leftMin > rightMin) ?
        peakVal - leftMin : peakVal - rightMin;

    // Special cases
    // Will avoid very low or zero prominence, for border partials
    if (leftIndex == minIndex)
        prominence = peakVal - rightMin;
    if (rightIndex == maxIndex)
        prominence = peakVal - leftMin;
    
    peak->mProminence = prominence;
}

void
PeakDetectorBillauer::ComputePeaksProminence(const WDL_TypedBuf<BL_FLOAT> &data,
                                             vector<Peak> *peaks,
                                             int minIndex, int maxIndex)
{
    // Compute peaks prominence
    for (int i = 0; i < peaks->size(); i++)
    {
        Peak &peak = (*peaks)[i];
        ComputePeakProminence(data, &peak, minIndex, maxIndex);
    }
}

bool
PeakDetectorBillauer::DBG_TestPeaks(const WDL_TypedBuf<BL_FLOAT> &data,
                                    const vector<Peak> &peaks)
{
    for (int i = 0; i < peaks.size(); i++)
    {
        const Peak &peak = peaks[i];

        BL_FLOAT peakAmp = data.Get()[peak.mPeakIndex];

        BL_FLOAT peakAmp0 = peakAmp;
        if (i - 1 >= 0)
            peakAmp0 = data.Get()[peak.mPeakIndex - 1];

        BL_FLOAT peakAmp1 = peakAmp;
        if (i + 1 < peaks.size())
            peakAmp1 = data.Get()[peak.mPeakIndex + 1];

        if ((peakAmp0 > peakAmp) || (peakAmp1 > peakAmp))
            return false;
    }

    return true;
}

void
PeakDetectorBillauer::DBG_PrintPeaks(const WDL_TypedBuf<BL_FLOAT> &data,
                                    const vector<Peak> &peaks)
{
    BLDebug::DumpData("peaks-data.txt", data);
    
    for (int i = 0; i < peaks.size(); i++)
    {
        const Peak &peak = peaks[i];
        fprintf(stderr, "indices: [%d %d %d]\n",
                peak.mLeftIndex, peak.mPeakIndex, peak.mRightIndex);
    }

    fprintf(stderr, "\n");
}

void
PeakDetectorBillauer::DBG_DumpPeaks(const WDL_TypedBuf<BL_FLOAT> &data,
                                    const vector<Peak> &peaks)
{
    BLDebug::DumpData("data.txt", data);
    
    WDL_TypedBuf<BL_FLOAT> peaksData;
    peaksData.Resize(data.GetSize());
    BLUtils::FillAllZero(&peaksData);

    WDL_TypedBuf<BL_FLOAT> peaksDataLarge;
    peaksDataLarge.Resize(data.GetSize());
    BLUtils::FillAllZero(&peaksDataLarge);

    WDL_TypedBuf<BL_FLOAT> peaksProminence;
    peaksProminence.Resize(data.GetSize());
    BLUtils::FillAllZero(&peaksProminence);
    
    for (int i = 0; i < peaks.size(); i++)
    {
        const Peak &peak = peaks[i];

        BL_FLOAT val = data.Get()[peak.mPeakIndex];
        peaksData.Get()[peak.mPeakIndex] = val;

        for (int j = peak.mLeftIndex; j <= peak.mRightIndex; j++)
        {
            BL_FLOAT val2 = data.Get()[j];
            peaksDataLarge.Get()[j] = val2;
        }

        BL_FLOAT prominence = peak.mProminence;
        peaksProminence.Get()[peak.mPeakIndex] = prominence;
    }

    BLDebug::DumpData("peaks.txt", peaksData);
    BLDebug::DumpData("peaks-large.txt", peaksDataLarge);
    BLDebug::DumpData("peaks-prominence.txt", peaksProminence);
}
