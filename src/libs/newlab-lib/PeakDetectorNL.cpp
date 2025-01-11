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

#include <BLUtilsMath.h>

#include "PeakDetectorBL.h"

#define DISCARD_INVALID_PEAKS 1

PeakDetectorBL::PeakDetectorBL() {}

PeakDetectorBL::~PeakDetectorBL() {}

void
PeakDetectorBL::DetectPeaks(const WDL_TypedBuf<BL_FLOAT> &data,
                            vector<Peak> *peaks,
                            int minIndex, int maxIndex)
{
    peaks->clear();
    
    // prevIndex, currentIndex, nextIndex
    
    // Skip the first ones
    // (to avoid artifacts of very low freq partial)
    if (minIndex < 0)
        minIndex = 0;
    if (maxIndex < 0)
        maxIndex = data.GetSize() - 1;

    BL_FLOAT prevVal = 0.0;
    BL_FLOAT nextVal = 0.0;
    BL_FLOAT currentVal = 0.0;

    int currentIndex = minIndex;
    while(currentIndex < maxIndex)
    {
        if ((currentVal > prevVal) && (currentVal >= nextVal))
        // Maximum found
        {
            if (currentIndex - 1 >= 0)
            {
                // Take the left and right "feets" of the partial,
                // then the middle.
                // (in order to be more precise)
                
                // Left
                int leftIndex = currentIndex;
                if (leftIndex > 0)
                {
                    BL_FLOAT prevLeftVal = data.Get()[leftIndex];
                    while(leftIndex > 0)
                    {
                        leftIndex--;
                        
                        BL_FLOAT leftVal = data.Get()[leftIndex];
                        
                        // Stop if we reach 0 or if it goes up again
                        if ((leftVal < BL_EPS) || (leftVal > prevLeftVal))
                        {
                            if (leftVal >= prevLeftVal)
                                leftIndex++;
                            
                            // Check bounds
                            if (leftIndex < 0)
                                leftIndex = 0;
                            
                            if (leftIndex > maxIndex)
                                leftIndex = maxIndex;
                            
                            break;
                        }
                        
                        prevLeftVal = leftVal;
                    }
                }
                
                // Right
                int rightIndex = currentIndex;
                
                if (rightIndex <= maxIndex)
                {
                    BL_FLOAT prevRightVal = data.Get()[rightIndex];
                    
                    while(rightIndex < maxIndex)
                    {
                        rightIndex++;
                                
                        BL_FLOAT rightVal = data.Get()[rightIndex];
                                
                        // Stop if we reach 0 or if it goes up again
                        if ((rightVal < BL_EPS) || (rightVal > prevRightVal))
                        {
                            if (rightVal >= prevRightVal)
                                rightIndex--;
                                    
                            // Check bounds
                            if (rightIndex < 0)
                                rightIndex = 0;
                                    
                            if (rightIndex > maxIndex)
                                rightIndex = maxIndex;
                                    
                            break;
                        }
                                
                        prevRightVal = rightVal;
                    }
                }
                
                // Take the max (better than taking the middle)
                int peakIndex = currentIndex;
                
                if ((peakIndex < 0) || (peakIndex > maxIndex))
                // Out of bounds
                    continue;
                
                bool discard = false;
                    
#if DISCARD_INVALID_PEAKS
                if (!discard)
                {
                    discard = DiscardInvalidPeaks(data, peakIndex,
                                                  leftIndex, rightIndex);
                }
#endif
                
                if (!discard)
                {
                    // Create new peak
                    //
                    Peak p;
                    p.mPeakIndex = peakIndex;
                    p.mLeftIndex = leftIndex;
                    p.mRightIndex = rightIndex;

                    peaks->push_back(p);
                }
                
                // Go just after the right foot of the partial
                currentIndex = rightIndex;
            }
        }
        else
            // No maximum found, continue 1 step
            currentIndex++;
        
        // Update the values
        currentVal = data.Get()[currentIndex];
        
        if (currentIndex - 1 >= 0)
            prevVal = data.Get()[currentIndex - 1];
        
        if (currentIndex + 1 <= maxIndex)
            nextVal = data.Get()[currentIndex + 1];
    }
}

bool
PeakDetectorBL::DiscardInvalidPeaks(const WDL_TypedBuf<BL_FLOAT> &data,
                                    int peakIndex, int leftIndex, int rightIndex)
{
    BL_FLOAT peakAmp = data.Get()[peakIndex];
    BL_FLOAT leftAmp = data.Get()[leftIndex];
    BL_FLOAT rightAmp = data.Get()[rightIndex];
    
    if ((peakAmp > leftAmp) && (peakAmp > rightAmp))
        // Correct, do not discard
        return false;
    
    return true;
}
