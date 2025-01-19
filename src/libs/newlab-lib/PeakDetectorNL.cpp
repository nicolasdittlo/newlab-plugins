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

#include "Defines.h"
#include "PeakDetectorNL.h"


PeakDetectorNL::PeakDetectorNL() {}

PeakDetectorNL::~PeakDetectorNL() {}

void
PeakDetectorNL::detectPeaks(const vector<float> &data,
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
        maxIndex = data.size() - 1;

    float prevVal = 0.0;
    float nextVal = 0.0;
    float currentVal = 0.0;

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
                    float prevLeftVal = data.data()[leftIndex];
                    while(leftIndex > 0)
                    {
                        leftIndex--;
                        
                        float leftVal = data.data()[leftIndex];
                        
                        // Stop if we reach 0 or if it goes up again
                        if ((leftVal < NL_EPS) || (leftVal > prevLeftVal))
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
                    float prevRightVal = data.data()[rightIndex];
                    
                    while(rightIndex < maxIndex)
                    {
                        rightIndex++;
                                
                        float rightVal = data.data()[rightIndex];
                                
                        // Stop if we reach 0 or if it goes up again
                        if ((rightVal < NL_EPS) || (rightVal > prevRightVal))
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
                    
                if (!discard)
                {
                    discard = discardInvalidPeaks(data, peakIndex,
                                                  leftIndex, rightIndex);
                }
                
                if (!discard)
                {
                    // Create new peak
                    //
                    Peak p;
                    p._peakIndex = peakIndex;
                    p._leftIndex = leftIndex;
                    p._rightIndex = rightIndex;

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
        currentVal = data.data()[currentIndex];
        
        if (currentIndex - 1 >= 0)
            prevVal = data.data()[currentIndex - 1];
        
        if (currentIndex + 1 <= maxIndex)
            nextVal = data.data()[currentIndex + 1];
    }
}

bool
PeakDetectorNL::discardInvalidPeaks(const vector<float> &data,
                                    int peakIndex, int leftIndex, int rightIndex)
{
    float peakAmp = data.data()[peakIndex];
    float leftAmp = data.data()[leftIndex];
    float rightAmp = data.data()[rightIndex];
    
    if ((peakAmp > leftAmp) && (peakAmp > rightAmp))
        // Correct, do not discard
        return false;
    
    return true;
}
