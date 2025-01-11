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

#ifndef PEAK_DETECTOR_H
#define PEAK_DETECTOR_H

#include <vector>
using namespace std;

class PeakDetector
{
 public:
    struct Peak
    {
        int mPeakIndex;
        int mLeftIndex;
        int mRightIndex;

        // Optional
        BL_FLOAT mProminence;

        static bool ProminenceLess(const Peak &p1, const Peak &p2)
        { return (p1.mProminence < p2.mProminence); }

        static bool PeakIndexLess(const Peak &p1, const Peak &p2)
        { return (p1.mPeakIndex < p2.mPeakIndex); }
    };

    virtual void SetThreshold(BL_FLOAT threshold) {}
    virtual void SetThreshold2(BL_FLOAT threshold2) {}
    
    virtual void DetectPeaks(const WDL_TypedBuf<BL_FLOAT> &data, vector<Peak> *peaks,
                             int minIndex = -1, int maxIndex = -1) = 0;
};

#endif
