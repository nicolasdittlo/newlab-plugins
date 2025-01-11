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

#include <vector>
using namespace std;

#include "PeakDetector.h"

// See: http://billauer.co.il/peakdet.html
// And: https://github.com/xuphys/peakdetect/blob/master/peakdetect.c
class PeakDetectorBillauer : public PeakDetector
{
 public:
    // Can use norm dB scale, or else pure log scale 
    PeakDetectorBillauer(float maxDelta);
    virtual ~PeakDetectorBillauer();

    void setThreshold(float threshold) override;
    void setThreshold2(float threshold2) override;
    
    void detectPeaks(const vector<float> &data, vector<Peak> *peaks,
                     int minIndex = -1, int maxIndex = -1) override;

protected:
    void suppressSmallPeaksSimple(const vector<float> &data,
                                  vector<Peak> *peaks);
    // Remove peaks with small prominance
    void suppressSmallPeaksProminence(const vector<float> &data,
                                      vector<Peak> *peaks,
                                      int minIndex, int maxIndex);
    // Remove high harmonics
    void suppressSmallPeaksFrequency(const vector<float> &data,
                                     vector<Peak> *peaks,
                                     int minIndex, int maxIndex);
    
    // Naive algorithm
    void adjustPeaksWidthSimple(const vector<float> &data,
                                vector<Peak> *peaks, int minIndex, int maxIndex);

    void adjustPeaksWidthProminence(const vector<float> &data,
                                    vector<Peak> *peaks,
                                    int minIndex, int maxIndex);
    
    void computePeakProminenceSimple(const vector<float> &data, Peak *peak);

    // Real prominence
    void computePeakProminence(const vector<float> &data, Peak *peak,
                               int minIndex, int maxIndex);

    void computePeaksProminence(const vector<float> &data,
                                vector<Peak> *peaks,
                                int minIndex, int maxIndex);
                                
    
    float mMaxDelta;
    
    float mDelta;

    float mThreshold2;
};

#endif
