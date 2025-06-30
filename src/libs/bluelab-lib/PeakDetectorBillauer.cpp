/* Copyright (C) 2025 Nicolas Dittlo <bluelab.plugins@gmail.com>
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

#include <algorithm>
using namespace std;

#include "Defines.h"
#include "Utils.h"
#include "PeakDetectorBillauer.h"

// See also: Matlab peak detection (of prominence and this kind of stuff)
// https://fr.mathworks.com/help/signal/ref/findpeaks.html

// Keep 20 peaks and suppress only if more than 20 peaks 
#define SUPPRESS_MIN_NUM_PEAKS 20.0

PeakDetectorBillauer::PeakDetectorBillauer(float maxDelta)
{    
    _maxDelta = maxDelta;

    float threshold = 0.01;
    
    _delta = threshold*_maxDelta;

    _threshold2 = 1.0;
}

PeakDetectorBillauer::~PeakDetectorBillauer() {}

void
PeakDetectorBillauer::setThreshold(float threshold)
{   
    _delta = threshold*_maxDelta;
}

void
PeakDetectorBillauer::setThreshold2(float threshold2)
{
    _threshold2 = threshold2;
}

void
PeakDetectorBillauer::
detectPeaks(const vector<float> &data, vector<Peak> *peaks,
            int minIndex, int maxIndex)
{
    // Arguments
    if (minIndex < 0)
        minIndex = 0;
    if (maxIndex < 0)
        maxIndex = data.size() - 1;
    
    // First, fill the min and max arrays
    
    vector<int> mintab;
    vector<int> maxtab;

    float mn = BL_INF;
    float mx = -BL_INF;
    int mnpos = -1;
    int mxpos = -1;

    // Look for max first
    bool lookformax = true;

    // Check if we start by a peak
    bool startedbypeak = false;
    if (maxIndex - minIndex >= 2)
    {
        float val0 = data.data()[minIndex];
        float val1 = data.data()[minIndex + 1];

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
        float t = data.data()[i];
        
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
            if (t < mx - _delta)
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
            if (t > mn + _delta)
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
            if (!(data.data()[maxtab[0]] >= data.data()[mintab[0]] + _delta))
                keepfirstpeak = false;
        }
    }
    
    // Secondly, fill the peaks
    
    peaks->clear();
    for (int i = 0; i < maxtab.size(); i++)
    {
        if ((i == 0) && !keepfirstpeak)
            continue;
        
        Peak peak;
        peak._peakIndex = maxtab[i];
        peak._leftIndex = ((i - 1 >= 0) && (i - 1 < mintab.size())) ?
            mintab[i - 1] : minIndex;
        peak._rightIndex = (i < mintab.size()) ? mintab[i] : maxIndex;
        
        peaks->push_back(peak);
    }
    
    // Post process
    
    // Simple, does not enlarge too much
    adjustPeaksWidthSimple(data, peaks, minIndex, maxIndex);
    
    suppressSmallPeaksFrequency(data, peaks, minIndex, maxIndex);
}

void
PeakDetectorBillauer::suppressSmallPeaksFrequency(const vector<float> &data,
                                                  vector<Peak> *peaks,
                                                  int minIndex, int maxIndex)
{
    if (_threshold2 >= 1.0)
        // Take all peaks
        return;

    if (peaks->size() < SUPPRESS_MIN_NUM_PEAKS)
        return;

    // Sort peaks by prominence
    sort(peaks->begin(), peaks->end(), Peak::peakIndexLess);

    // Keep only the biggest peaks
    int numToTakePeaks = peaks->size()*_threshold2;

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
PeakDetectorBillauer::adjustPeaksWidthSimple(const vector<float> &data,
                                             vector<Peak> *peaks,
                                             int minIndex, int maxIndex)
{    
    for (int i = 0; i < peaks->size(); i++)
    {
        Peak &peak = (*peaks)[i];

        float peakAmp = data.data()[peak._peakIndex];

        // Use prominence
#define PEAKS_WIDTH_RATIO2 0.75 
        computePeakProminenceSimple(data, &peak);
        float thrs = peakAmp - fabs(peak._prominence*PEAKS_WIDTH_RATIO2);
        
        // Make sure not to enlarge more than prev Billauer minima
        int originLeftIndex = peak._leftIndex;
        int originRightIndex = peak._rightIndex;
        
        // Adjust left index
        for (int j = peak._peakIndex - 1; j >= minIndex; j--)
        {
            if (j <= originLeftIndex)
                break;
            
            float a = data.data()[j];
            
            if (a < thrs)
            {
                peak._leftIndex = j;
                
                break;
            }
        }

        // Adjust right index
        for (int j = peak._peakIndex + 1; j <= maxIndex; j++)
        {
            if (j >= originRightIndex)
                break;
            
            float a = data.data()[j];

            // Test decrease
            if (a < thrs)
            {
                peak._rightIndex = j;
                
                break;
            }
        }

        // Avoid very asymetrical peaks
        // => make all peaks symetric, using minimal width distance
        int leftWidth = peak._peakIndex - peak._leftIndex;
        int rightWidth = peak._rightIndex - peak._peakIndex;
        if (leftWidth > rightWidth)
            peak._leftIndex = peak._peakIndex - rightWidth;
        else if (rightWidth > leftWidth)
            peak._rightIndex = peak._peakIndex + leftWidth;
    }
}

// Prominence is computed from the highest border
//
// See: https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.peak_prominences.html#scipy.signal.peak_prominences
void
PeakDetectorBillauer::computePeakProminenceSimple(const vector<float> &data,
                                                  Peak *peak)
{
    float lm = data.data()[peak->_leftIndex];
    float rm = data.data()[peak->_rightIndex];

    float base = (lm > rm) ? lm : rm;

    peak->_prominence = data.data()[peak->_peakIndex] - base;
}

// Real prominence. 
// See algorithm here:
// https://fr.mathworks.com/help/signal/ug/prominence.html#d123e28940
void
PeakDetectorBillauer::computePeakProminence(const vector<float> &data,
                                            Peak *peak, int minIndex, int maxIndex)
{
    // Extend a horizontal line from the peak to the left and right until
    // the line does one of the following:
    // - Crosses the signal because there is a higher peak
    // - Reaches the left or right end of the signal
    
    float peakVal = data.data()[peak->_peakIndex];
        
    int leftIndex = peak->_peakIndex;
    int rightIndex = peak->_peakIndex;
    
    for (int j = peak->_peakIndex; j >= minIndex; j--)
    {
        float val = data.data()[j];
        if (val > peakVal)
            break;
        
        leftIndex = j;
    }
    
    for (int j = peak->_peakIndex; j <= maxIndex; j++)
    {
        float val = data.data()[j];
        if (val > peakVal)
            break;
        
        rightIndex = j;
    }
    
    //Find the minimum of the signal in each of the two intervals defined
    // in Step 2. This point is either a valley or one of the signal endpoints.
    float leftMin = peakVal;
    float rightMin = peakVal;
    
    for (int j = peak->_peakIndex; j >= leftIndex; j--)
    {
        float val = data.data()[j];
        
        if (val < leftMin)
            leftMin = val;
    }
    
    for (int j = peak->_peakIndex; j <= rightIndex; j++)
    {
        float val = data.data()[j];
        
        if (val < rightMin)
            rightMin = val;
    }
    
    // The higher of the two interval minima specifies the reference level.
    // The height of the peak above this level is its prominence.
    
    float prominence = (leftMin > rightMin) ?
        peakVal - leftMin : peakVal - rightMin;

    // Special cases
    // Will avoid very low or zero prominence, for border partials
    if (leftIndex == minIndex)
        prominence = peakVal - rightMin;
    if (rightIndex == maxIndex)
        prominence = peakVal - leftMin;
    
    peak->_prominence = prominence;
}

void
PeakDetectorBillauer::computePeaksProminence(const vector<float> &data,
                                             vector<Peak> *peaks,
                                             int minIndex, int maxIndex)
{
    // Compute peaks prominence
    for (int i = 0; i < peaks->size(); i++)
    {
        Peak &peak = (*peaks)[i];
        computePeakProminence(data, &peak, minIndex, maxIndex);
    }
}
