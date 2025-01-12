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

#define MAX_ZOMBIE_AGE 2

#define DELTA_FREQ_ASSOC 0.01 // For normalized freqs. Around 100Hz

#define PARTIALS_HISTORY_SIZE 2

PartialFilterMarchand::PartialFilterMarchand(int bufferSize, float sampleRate)
{
    _bufferSize = bufferSize;
}

PartialFilterMarchand::~PartialFilterMarchand() {}

void
PartialFilterMarchand::reset(int bufferSize, float sampleRate)
{
    _bufferSize = bufferSize;

    _partials.clear();
}
           
void
PartialFilterMarchand::filterPartials(vector<Partial> *partials)
{    
    _partials.push_front(*partials);
    
    while(_partials.size() > PARTIALS_HISTORY_SIZE)
        _partials.pop_back();
    
    partials->clear();
    
    if (_partials.empty())
        return;
    
    if (_partials.size() == 1)
        // Assigne ids to the first series of partials
    {
        for (int j = 0; j < _partials[0].size(); j++)
        {
            Partial &currentPartial = _partials[0][j];
            currentPartial.genNewId();
        }
        
        // Not enough partials to filter, need 2 series
        return;
    }
    
    if (_partials.size() < 2)
        return;
    
    const vector<Partial> &prevPartials = _partials[1];
    vector<Partial> &currentPartials = _tmpPartials0;
    currentPartials = _partials[0];
    
    // Partials that was not associated at the end
    vector<Partial> &remainingPartials = _tmpPartials1;
    remainingPartials.resize(0);
    
    associatePartialsPARSHL(prevPartials, &currentPartials, &remainingPartials);
    
    // Add the new zombie and dead partials
    for (int i = 0; i < prevPartials.size(); i++)
    {
        const Partial &prevPartial = prevPartials[i];

        bool found = false;
        for (int j = 0; j < currentPartials.size(); j++)
        {
            const Partial &currentPartial = currentPartials[j];
            
            if (currentPartial._id == prevPartial._id)
            {
                found = true;
                
                break;
            }
        }

        if (!found)
        {
            if (prevPartial._state == Partial::ALIVE)
            {
                // We set zombie for 1 frame only
                Partial newPartial = prevPartial;
                newPartial._state = Partial::ZOMBIE;
                newPartial._zombieAge = 0;
                
                // Kalman:
                // Extrapolate the zombies
                newPartial._freq = newPartial._kf.updateEstimate(newPartial._freq);
                
                currentPartials.push_back(newPartial);
            }
            else if (prevPartial._state == Partial::ZOMBIE)
            {
                Partial newPartial = prevPartial;
                
                newPartial._zombieAge++;
                if (newPartial._zombieAge >= MAX_ZOMBIE_AGE)
                    newPartial._state = Partial::DEAD;
  
                // Kalman
                // extrapolate the zombies
                newPartial._freq = newPartial._kf.updateEstimate(newPartial._freq);

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
        
        p.genNewId();
        
        currentPartials.push_back(p);
    }
    
    // Then sort the new partials by frequency
    sort(currentPartials.begin(), currentPartials.end(), Partial::freqLess);
    
    //
    // Update: add the partials to the history
    // (except the dead ones)
    _partials[0].clear();
    for (int i = 0; i < currentPartials.size(); i++)
    {
        const Partial &currentPartial = currentPartials[i];
        
        // Do not skip the dead partials: they will be used for fade out !
        //if (currentPartial.mState != Partial::DEAD)
        _partials[0].push_back(currentPartial);
    }

    *partials = _partials[0];
}

// Use method similar to SAS
void
PartialFilterMarchand::
associatePartials(const vector<Partial> &prevPartials,
                  vector<Partial> *currentPartials,
                  vector<Partial> *remainingPartials)
{
    // Sort current partials and prev partials by decreasing amplitude
    vector<Partial> &currentPartialsSort = _tmpPartials2;
    currentPartialsSort = *currentPartials;
    sort(currentPartialsSort.begin(), currentPartialsSort.end(), Partial::ampLess);
    reverse(currentPartialsSort.begin(), currentPartialsSort.end());
    
    vector<Partial> &prevPartialsSort = _tmpPartials3;
    prevPartialsSort = prevPartials;
    
    sort(prevPartialsSort.begin(), prevPartialsSort.end(), Partial::ampLess);
    reverse(prevPartialsSort.begin(), prevPartialsSort.end());
 
    // Associate
    
    // Associated partials
    vector<Partial> &currentPartialsAssoc = _tmpPartials4;
    currentPartialsAssoc.resize(0);
    
    for (int i = 0; i < prevPartialsSort.size(); i++)
    {
        const Partial &prevPartial = prevPartialsSort[i];
        
        for (int j = 0; j < currentPartialsSort.size(); j++)
        {
            Partial &currentPartial = currentPartialsSort[j];
            
            if (currentPartial._id != -1)
                // Already assigned
                continue;
            
            float diffFreq = fabs(prevPartial._freq - currentPartial._freq);
            
            int binNum = currentPartial._freq*_bufferSize*0.5;
            float diffCoeff = getDeltaFreqCoeff(binNum);
            if (diffFreq < DELTA_FREQ_ASSOC*diffCoeff)
            // Associated !
            {
                currentPartial._id = prevPartial._id;
                currentPartial._state = Partial::ALIVE;
                currentPartial._wasAlive = true;
                
                currentPartial._age = prevPartial._age + 1;
            
                // Kalman
                currentPartial._kf = prevPartial._kf;
                currentPartial._freq =
                    currentPartial._kf.updateEstimate(currentPartial._freq);

                currentPartialsAssoc.push_back(currentPartial);
                
                // We have associated to the prev partial
                // We are done!
                // Stop the search here.
                break;
            }
        }
    }
    
    sort(currentPartialsAssoc.begin(), currentPartialsAssoc.end(), Partial::idLess);
     *currentPartials = currentPartialsAssoc;
    
    // Add the remaining partials
    remainingPartials->clear();
    for (int i = 0; i < currentPartialsSort.size(); i++)
    {
        const Partial &p = currentPartialsSort[i];
        if (p._id == -1)
            remainingPartials->push_back(p);
    }
}

// Use PARSHL method
void
PartialFilterMarchand::
associatePartialsPARSHL(const vector<Partial> &prevPartials,
                        vector<Partial> *currentPartials,
                        vector<Partial> *remainingPartials)
{
    // Sort current partials and prev partials by increasing frequency
    sort(currentPartials->begin(), currentPartials->end(), Partial::freqLess);
    
    vector<Partial> &prevPartials0 = _tmpPartials5;
    prevPartials0 = prevPartials;
    sort(prevPartials0.begin(), prevPartials0.end(), Partial::freqLess);
    
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
                if (currentPartial._id != -1)
                    // Already associated, nothing to do on this step!
                    continue;
                
                float diffFreq =
                    std::fabs(prevPartial._freq - currentPartial._freq);
                
                int binNum = currentPartial._freq*_bufferSize*0.5;
                float diffCoeff = getDeltaFreqCoeff(binNum);
            
                if (diffFreq < DELTA_FREQ_ASSOC*diffCoeff)
                    // Associate!
                {
                    int otherIdx =
                        findPartialById(*currentPartials, (int)prevPartial._id);
                    
                    if (otherIdx == -1)
                        // This partial is not yet associated
                        // => No fight
                    {
                        currentPartial._id = prevPartial._id;
                        currentPartial._age = prevPartial._age;
                        currentPartial._kf = prevPartial._kf;
                        
                        stopFlag = false;
                    }
                    else // Fight!
                    {
                        Partial &otherPartial = (*currentPartials)[otherIdx];
                        
                        float otherDiffFreq = fabs(prevPartial._freq - otherPartial._freq);
                        
                        if (diffFreq < otherDiffFreq)
                        // Current partial won
                        {
                            currentPartial._id = prevPartial._id;
                            currentPartial._age = prevPartial._age;
                            currentPartial._kf = prevPartial._kf;
                            
                            // Detach the other
                            otherPartial._id = -1;
                            
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
    vector<Partial> &newPartials = _tmpPartials6;
    newPartials.resize(0);
    
    for (int j = 0; j < currentPartials->size(); j++)
    {
        Partial &currentPartial = (*currentPartials)[j];
        
        if (currentPartial._id != -1)
        {
            currentPartial._state = Partial::ALIVE;
            currentPartial._wasAlive = true;
    
            // Increment age
            currentPartial._age = currentPartial._age + 1;
            currentPartial._freq =
                currentPartial._kf.updateEstimate(currentPartial._freq);
    
            newPartials.push_back(currentPartial);
        }
    }
    
    // Add the remaining partials
    remainingPartials->clear();
    for (int i = 0; i < currentPartials->size(); i++)
    {
        const Partial &p = (*currentPartials)[i];
        if (p._id == -1)
            remainingPartials->push_back(p);
    }
    
    // Update current partials
    *currentPartials = newPartials;
}

float
PartialFilterMarchand::getDeltaFreqCoeff(int binNum)
{
#define END_COEFF 0.25
    
    float t = ((float)binNum)/(_bufferSize*0.5);
    float diffCoeff = 1.0 - (1.0 - END_COEFF)*t;
    
    return diffCoeff;
}

int
PartialFilterMarchand::findPartialById(const vector<Partial> &partials, int idx)
{
    for (int i = 0; i < partials.size(); i++)
    {
        const Partial &partial = partials[i];
        
        if (partial._id == idx)
            return i;
    }
    
    return -1;
}
