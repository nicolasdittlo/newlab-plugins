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

#ifndef PEAK_DETECTOR_BILLAUER_H
#define PEAK_DETECTOR_BILLAUER_H

#include <PeakDetector.h>

// See: http://billauer.co.il/peakdet.html
// And: https://github.com/xuphys/peakdetect/blob/master/peakdetect.c
class PeakDetectorBillauer : public PeakDetector
{
 public:
    // Can use norm dB scale, or else pure log scale 
    PeakDetectorBillauer(BL_FLOAT maxDelta);
    virtual ~PeakDetectorBillauer();

    void SetThreshold(BL_FLOAT threshold) override;
    void SetThreshold2(BL_FLOAT threshold2) override;
    
    void DetectPeaks(const WDL_TypedBuf<BL_FLOAT> &data, vector<Peak> *peaks,
                     int minIndex = -1, int maxIndex = -1) override;

protected:
    void SuppressSmallPeaksSimple(const WDL_TypedBuf<BL_FLOAT> &data,
                                  vector<Peak> *peaks);
    // Remove peaks with small prominance
    void SuppressSmallPeaksProminence(const WDL_TypedBuf<BL_FLOAT> &data,
                                      vector<Peak> *peaks,
                                      int minIndex, int maxIndex);
    // Remove high harmonics
    void SuppressSmallPeaksFrequency(const WDL_TypedBuf<BL_FLOAT> &data,
                                     vector<Peak> *peaks,
                                     int minIndex, int maxIndex);
    
    // Naive algorithm
    void AdjustPeaksWidthSimple(const WDL_TypedBuf<BL_FLOAT> &data,
                                vector<Peak> *peaks, int minIndex, int maxIndex);

    void AdjustPeaksWidthProminence(const WDL_TypedBuf<BL_FLOAT> &data,
                                    vector<Peak> *peaks,
                                    int minIndex, int maxIndex);

    //void AdjustPeaksWidthNoisy(const WDL_TypedBuf<BL_FLOAT> &data,
    //                           vector<Peak> *peaks);
    
    void ComputePeakProminenceSimple(const WDL_TypedBuf<BL_FLOAT> &data, Peak *peak);

    // Real prominence
    void ComputePeakProminence(const WDL_TypedBuf<BL_FLOAT> &data, Peak *peak,
                               int minIndex, int maxIndex);

    void ComputePeaksProminence(const WDL_TypedBuf<BL_FLOAT> &data,
                                vector<Peak> *peaks,
                                int minIndex, int maxIndex);
                                
    bool DBG_TestPeaks(const WDL_TypedBuf<BL_FLOAT> &data, const vector<Peak> &peaks);
    // Print peak
    void DBG_PrintPeaks(const WDL_TypedBuf<BL_FLOAT> &data, const vector<Peak> &peaks);
    // Dump array, showing peak bounds
    void DBG_DumpPeaks(const WDL_TypedBuf<BL_FLOAT> &data,
                       const vector<Peak> &peaks);
        
    //
    BL_FLOAT mMaxDelta;
    
    BL_FLOAT mDelta;

    BL_FLOAT mThreshold2;
};

#endif
