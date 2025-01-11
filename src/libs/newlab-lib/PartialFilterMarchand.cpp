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

#include <stdio.h>

#include <cmath>
#include <algorithm>
using namespace std;

#include "PartialFilterMarchand.h"

// Seems better without, not sure...
#define USE_KALMAN_FOR_ASSOC 0 //1 //0

#define MAX_ZOMBIE_AGE 2

// Seems better with 200Hz (tested on "oohoo")
#define DELTA_FREQ_ASSOC 0.01 // For normalized freqs. Around 100Hz

#define PARTIALS_HISTORY_SIZE 2

PartialFilterMarchand::PartialFilterMarchand(int bufferSize, BL_FLOAT sampleRate)
{
    mBufferSize = bufferSize;
}

PartialFilterMarchand::~PartialFilterMarchand() {}

void
PartialFilterMarchand::Reset(int bufferSize, BL_FLOAT sampleRate)
{
    mBufferSize = bufferSize;

    mPartials.clear();
}
           
void
PartialFilterMarchand::FilterPartials(vector<Partial> *partials)
{    
    mPartials.push_front(*partials);
    
    while(mPartials.size() > PARTIALS_HISTORY_SIZE)
        mPartials.pop_back();
    
    partials->clear();
    
    if (mPartials.empty())
        return;
    
    if (mPartials.size() == 1)
        // Assigne ids to the first series of partials
    {
        for (int j = 0; j < mPartials[0].size(); j++)
        {
            Partial &currentPartial = mPartials[0][j];
            currentPartial.GenNewId();
        }
        
        // Not enough partials to filter, need 2 series
        return;
    }
    
    if (mPartials.size() < 2)
        return;
    
    const vector<Partial> &prevPartials = mPartials[1];
    vector<Partial> &currentPartials = mTmpPartials0;
    currentPartials = mPartials[0];
    
    // Partials that was not associated at the end
    vector<Partial> &remainingPartials = mTmpPartials1;
    remainingPartials.resize(0);
    
    AssociatePartialsPARSHL(prevPartials, &currentPartials, &remainingPartials);
    
    // Add the new zombie and dead partials
    for (int i = 0; i < prevPartials.size(); i++)
    {
        const Partial &prevPartial = prevPartials[i];

        bool found = false;
        for (int j = 0; j < currentPartials.size(); j++)
        {
            const Partial &currentPartial = currentPartials[j];
            
            if (currentPartial.mId == prevPartial.mId)
            {
                found = true;
                
                break;
            }
        }

        if (!found)
        {
            if (prevPartial.mState == Partial::ALIVE)
            {
                // We set zombie for 1 frame only
                Partial newPartial = prevPartial;
                newPartial.mState = Partial::ZOMBIE;
                newPartial.mZombieAge = 0;
                
                // Kalman:
                // GOOD: extrapolate the zombies
                //newPartial.mPredictedFreq =
                newPartial.mFreq = newPartial.mKf.updateEstimate(newPartial.mFreq);
                
                currentPartials.push_back(newPartial);
            }
            else if (prevPartial.mState == Partial::ZOMBIE)
            {
                Partial newPartial = prevPartial;
                
                newPartial.mZombieAge++;
                if (newPartial.mZombieAge >= MAX_ZOMBIE_AGE)
                    newPartial.mState = Partial::DEAD;
  
                // Kalman
                // GOOD: extrapolate the zombies
                //newPartial.mPredictedFreq =
                newPartial.mFreq = newPartial.mKf.updateEstimate(newPartial.mFreq);

                currentPartials.push_back(newPartial);
            }
            
            // If DEAD, do not add, forget it
        }
    }
    
    // Get the result here
    // So we get the partials that are well tracked over time
    *partials = currentPartials;
    
    // At the end, there remains the partial that have not been matched
    //
    // Add them at to the history for next time
    //
    for (int i = 0; i < remainingPartials.size(); i++)
    {
        Partial p = remainingPartials[i];
        
        p.GenNewId();
        
        currentPartials.push_back(p);
    }
    
    // Then sort the new partials by frequency
    sort(currentPartials.begin(), currentPartials.end(), Partial::FreqLess);
    
    //
    // Update: add the partials to the history
    // (except the dead ones)
    mPartials[0].clear();
    for (int i = 0; i < currentPartials.size(); i++)
    {
        const Partial &currentPartial = currentPartials[i];
        
        // TEST: do not skip the dead partials:
        // they will be used for fade out !
        //if (currentPartial.mState != Partial::DEAD)
        mPartials[0].push_back(currentPartial);
    }

    *partials = mPartials[0];
}

// Use method similar to SAS
void
PartialFilterMarchand::
AssociatePartials(const vector<Partial> &prevPartials,
                  vector<Partial> *currentPartials,
                  vector<Partial> *remainingPartials)
{
    // Sort current partials and prev partials by decreasing amplitude
    vector<Partial> &currentPartialsSort = mTmpPartials2;
    currentPartialsSort = *currentPartials;
    sort(currentPartialsSort.begin(), currentPartialsSort.end(), Partial::AmpLess);
    reverse(currentPartialsSort.begin(), currentPartialsSort.end());
    
    vector<Partial> &prevPartialsSort = mTmpPartials3;
    prevPartialsSort = prevPartials;
    
    sort(prevPartialsSort.begin(), prevPartialsSort.end(), Partial::AmpLess);
    reverse(prevPartialsSort.begin(), prevPartialsSort.end());
 
    // Associate
    
    // Associated partials
    vector<Partial> &currentPartialsAssoc = mTmpPartials4;
    currentPartialsAssoc.resize(0);
    
    for (int i = 0; i < prevPartialsSort.size(); i++)
    {
        const Partial &prevPartial = prevPartialsSort[i];
        
        for (int j = 0; j < currentPartialsSort.size(); j++)
        {
            Partial &currentPartial = currentPartialsSort[j];
            
            if (currentPartial.mId != -1)
                // Already assigned
                continue;
            
#if USE_KALMAN_FOR_ASSOC
            BL_FLOAT diffFreq =
                std::fabs(prevPartial.mPredictedFreq - currentPartial.mFreq);
#else
            BL_FLOAT diffFreq = std::fabs(prevPartial.mFreq - currentPartial.mFreq);
#endif
            
            int binNum = currentPartial.mFreq*mBufferSize*0.5;
            BL_FLOAT diffCoeff = GetDeltaFreqCoeff(binNum);
            if (diffFreq < DELTA_FREQ_ASSOC*diffCoeff)
            // Associated !
            {
                currentPartial.mId = prevPartial.mId;
                currentPartial.mState = Partial::ALIVE;
                currentPartial.mWasAlive = true;
                
                currentPartial.mAge = prevPartial.mAge + 1;
            
                // Kalman
                currentPartial.mKf = prevPartial.mKf;
                //currentPartial.mPredictedFreq =
                currentPartial.mFreq =
                    currentPartial.mKf.updateEstimate(currentPartial.mFreq);

                currentPartialsAssoc.push_back(currentPartial);
                
                // We have associated to the prev partial
                // We are done!
                // Stop the search here.
                break;
            }
        }
    }
    
    sort(currentPartialsAssoc.begin(), currentPartialsAssoc.end(), Partial::IdLess);
     *currentPartials = currentPartialsAssoc;
    
    // Add the remaining partials
    remainingPartials->clear();
    for (int i = 0; i < currentPartialsSort.size(); i++)
    {
        const Partial &p = currentPartialsSort[i];
        if (p.mId == -1)
            remainingPartials->push_back(p);
    }
}

// Use PARSHL method
void
PartialFilterMarchand::
AssociatePartialsPARSHL(const vector<Partial> &prevPartials,
                        vector<Partial> *currentPartials,
                        vector<Partial> *remainingPartials)
{
    // Sort current partials and prev partials by increasing frequency
    sort(currentPartials->begin(), currentPartials->end(), Partial::FreqLess);
    
    vector<Partial> &prevPartials0 = mTmpPartials5;
    prevPartials0 = prevPartials;
    sort(prevPartials0.begin(), prevPartials0.end(), Partial::FreqLess);
    
    // Associated partials
    bool stopFlag = true;
    do {
        stopFlag = true;
        
        for (int i = 0; i < prevPartials0.size(); i++)
        {
            const Partial &prevPartial = prevPartials0[i];
            for (int j = 0; j < currentPartials->size(); j++)
            {
                Partial &currentPartial = (*currentPartials)[j];
                if (currentPartial.mId != -1)
                    // Already associated, nothing to do on this step!
                    continue;
                
#if USE_KALMAN_FOR_ASSOC
                BL_FLOAT diffFreq =
                    std::fabs(prevPartial.mPredictedFreq - currentPartial.mFreq);
#else
                BL_FLOAT diffFreq =
                    std::fabs(prevPartial.mFreq - currentPartial.mFreq);
#endif
                int binNum = currentPartial.mFreq*mBufferSize*0.5;
                BL_FLOAT diffCoeff = GetDeltaFreqCoeff(binNum);
            
                if (diffFreq < DELTA_FREQ_ASSOC*diffCoeff)
                    // Associate!
                {
                    int otherIdx =
                        FindPartialById(*currentPartials, (int)prevPartial.mId);
                    
                    if (otherIdx == -1)
                        // This partial is not yet associated
                        // => No fight
                    {
                        currentPartial.mId = prevPartial.mId;
                        currentPartial.mAge = prevPartial.mAge;
                        currentPartial.mKf = prevPartial.mKf; //
                        
                        stopFlag = false;
                    }
                    else // Fight!
                    {
                        Partial &otherPartial = (*currentPartials)[otherIdx];
                        
#if USE_KALMAN_FOR_ASSOC
                        BL_FLOAT otherDiffFreq =
                                std::fabs(prevPartial.mPredictedFreq -
                                          otherPartial.mFreq);
#else
                        BL_FLOAT otherDiffFreq =
                            std::fabs(prevPartial.mFreq - otherPartial.mFreq);
#endif
                        
                        if (diffFreq < otherDiffFreq)
                        // Current partial won
                        {
                            currentPartial.mId = prevPartial.mId;
                            currentPartial.mAge = prevPartial.mAge;
                            currentPartial.mKf = prevPartial.mKf; //
                            
                            // Detach the other
                            otherPartial.mId = -1;
                            
                            stopFlag = false;
                        }
                        else
                        // Other partial won
                        {
                            // Just keep it like it is!
                        }
                    }
                }
            }
        }
    } while (!stopFlag);
    
    
    // Update partials
    vector<Partial> &newPartials = mTmpPartials6;
    newPartials.resize(0);
    
    for (int j = 0; j < currentPartials->size(); j++)
    {
        Partial &currentPartial = (*currentPartials)[j];
        
        if (currentPartial.mId != -1)
        {
            currentPartial.mState = Partial::ALIVE;
            currentPartial.mWasAlive = true;
    
            // Increment age
            currentPartial.mAge = currentPartial.mAge + 1;
            //currentPartial.mPredictedFreq =
            currentPartial.mFreq =
                currentPartial.mKf.updateEstimate(currentPartial.mFreq);
    
            newPartials.push_back(currentPartial);
        }
    }
    
    // Add the remaining partials
    remainingPartials->clear();
    for (int i = 0; i < currentPartials->size(); i++)
    {
        const Partial &p = (*currentPartials)[i];
        if (p.mId == -1)
            remainingPartials->push_back(p);
    }
    
    // Update current partials
    *currentPartials = newPartials;
}

BL_FLOAT
PartialFilterMarchand::GetDeltaFreqCoeff(int binNum)
{
#define END_COEFF 0.25
    
    BL_FLOAT t = ((BL_FLOAT)binNum)/(mBufferSize*0.5);
    BL_FLOAT diffCoeff = 1.0 - (1.0 - END_COEFF)*t;
    
    return diffCoeff;
}

int
PartialFilterMarchand::FindPartialById(const vector<Partial> &partials, int idx)
{
    for (int i = 0; i < partials.size(); i++)
    {
        const Partial &partial = partials[i];
        
        if (partial.mId == idx)
            return i;
    }
    
    return -1;
}
