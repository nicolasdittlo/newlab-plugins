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

#include "Utils.h"
#include "PartialFilterAMFM.h"

#define MAX_ZOMBIE_AGE 1 // 3 5

// Must keep history size >= 3, for FixPartialsCrossing
#define PARTIALS_HISTORY_SIZE 3

#define ASSOC_SIMPLE_AMFM 1
// Result almost similar to ASSOC_SIMPLE_AMFM
#define ASSOC_SIMPLE_NERI 0 // NOTE: don't forget the hack zetaA = 50.0

#define DISCARD_OPPOSITE_DIRECTION 0

// To be checked
#define HARD_OPTIM 1 //0

PartialFilterAMFM::PartialFilterAMFM(int bufferSize, float sampleRate)
{    
    _bufferSize = bufferSize;
    _sampleRate = sampleRate;

    _neriDelta = 0.2;
}

PartialFilterAMFM::~PartialFilterAMFM() {}

void
PartialFilterAMFM::reset(int bufferSize, float sampleRate)
{    
    _bufferSize = bufferSize;
    _sampleRate = sampleRate;
    
    _partials.clear();
}
           
void
PartialFilterAMFM::filterPartials(vector<Partial> *partials)
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
            currentPartial.GenNewId();
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
    vector<Partial> &remainingCurrentPartials = _tmpPartials1;
    remainingCurrentPartials.resize(0);
    
#if ASSOC_SIMPLE_AMFM
    associatePartialsAMFM(prevPartials, &currentPartials, &remainingCurrentPartials);
#endif

#if ASSOC_SIMPLE_NERI
    associatePartialsNeri(prevPartials, &currentPartials,
                          &remainingCurrentPartials);
#endif
    
    vector<Partial> &deadZombiePartials = _tmpPartials7;
    computeZombieDeadPartials(prevPartials, currentPartials, &deadZombiePartials);
    
    // Add zombie and dead partial
    for (int i = 0; i < deadZombiePartials.size(); i++)
        currentPartials.push_back(deadZombiePartials[i]);

    if (_partials.size() >= 3)
    {
        sort(_partials[1].begin(), _partials[1].end(), Partial::IdLess);
        sort(_partials[2].begin(), _partials[2].end(), Partial::IdLess);
    
        fixPartialsCrossing(_partials[2], _partials[1], &currentPartials);
    }
    
    // At the end, there remains the partial that have not been matched
    //
    // Add them at to the history for next time
    //
    for (int i = 0; i < remainingCurrentPartials.size(); i++)
    {
        Partial p = remainingCurrentPartials[i];
        
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

        if (currentPartial.mState != Partial::DEAD)
            _partials[0].push_back(currentPartial);
    }

    *partials = _partials[0];
}

void
PartialFilterAMFM::setNeriDelta(float delta)
{
    _neriDelta = delta;
}

void
PartialFilterAMFM::
associatePartialsAMFM(const vector<Partial> &prevPartials,
                      vector<Partial> *currentPartials,
                      vector<Partial> *remainingCurrentPartials)
{    
    // Sometimes need more than 5 (and less than 10)
    // When threshold is near 1%
    // Sometimes it never solves totally and would lead to infinite num iters
#define MAX_NUM_ITER 10 //4
    
    // Problem: we miss the highest freqs if != 2048
#define NUM_STEPS_LOOKUP 8 //4 //2048 // 128 //4

#if HARD_OPTIM
#define MAX_NUM_ITER 4
    
    // Problem: we miss the highest freqs is != 2048
#define NUM_STEPS_LOOKUP 4
#endif
    
    // Sort current partials and prev partials by increasing frequency
    sort(currentPartials->begin(), currentPartials->end(), Partial::freqLess);
    
    vector<Partial> &prevPartials0 = _tmpPartials5;
    prevPartials0 = prevPartials;

    sort(prevPartials0.begin(), prevPartials0.end(), Partial::freqLess);

    // Reset the links
    for (int i = 0; i < prevPartials0.size(); i++)
        prevPartials0[i]._linkedId = -1;
    for (int i = 0; i < currentPartials->size(); i++)
    {
        (*currentPartials)[i]._linkedId = -1;

        // Just in case
        (*currentPartials)[i].mId = -1;
    }
    
    // Associated partials
    bool stopFlag = true;
    int numIters = 0;
    
    do {
        stopFlag = true;

        numIters++;
        
        for (int i = 0; i < prevPartials0.size(); i++)
        {
            Partial &prevPartial = prevPartials0[i];

            // Check if the link is already done
            if (((int)prevPartial.mId != -1) &&
                (prevPartial.mLinkedId != -1) &&
                ((*currentPartials)[prevPartial._linkedId]._linkedId == i))
                // Already linked
                continue;
            
            int nearestFreqId =
                findNearestFreqId(*currentPartials, prevPartial._freq, i);
            
            for (int j = nearestFreqId - NUM_STEPS_LOOKUP/2;
                 j < nearestFreqId + NUM_STEPS_LOOKUP/2; j++)
            {
                if ((j < 0) || (j >= currentPartials->size()))
                    continue;
                
                Partial &currentPartial = (*currentPartials)[j];
                
                if (currentPartial._id == prevPartial._id)
                    continue;
                
                bool discard = checkDiscardBigJump(prevPartial, currentPartial);
                if (discard)
                    continue;
                
                float LA = computeLA(prevPartial, currentPartial);
                float LF = computeLF(prevPartial, currentPartial);
                
                // As is the paper
                if ((LA > 0.5) && (LF > 0.5))
                    // Associate!
                {
                    // Current partial already has an id
                    bool mustFight0 = (currentPartial._id != -1);

                    int fight1Idx = prevPartial._linkedId;
                    
                    // Prev partial already has some association with the current id
                    bool mustFight1 = (fight1Idx != -1);
                        
                    if (!mustFight0 && !mustFight1)
                    {
                        currentPartial._id = prevPartial._id;
                        currentPartial._age = prevPartial._age;

                        currentPartial._linkedId = i;
                        prevPartial._linkedId = j;
                            
                        stopFlag = false;
                        
                        continue;
                    }
                    
                    // Fight!
                    //
                    
                    // Find the previous link for case 0
                    int otherPrevIdx = currentPartial._linkedId;

                    if ((otherPrevIdx == -1) && mustFight0)
                        continue;

                    if ((fight1Idx == -1) && !mustFight0)
                        continue;
                    
                    // Find prev partial
                    Partial prevPartialFight =
                        mustFight0 ? prevPartials0[otherPrevIdx] : prevPartial;
                    // Find current partial
                    Partial currentPartialFight =
                        mustFight0 ? currentPartial : (*currentPartials)[fight1Idx];

                    // Compute scores
                    float otherLA =
                        computeLA(prevPartialFight, currentPartialFight);
                    float otherLF =
                        computeLF(prevPartialFight, currentPartialFight);
                    
                    // Joint likelihood
                    float j0 = LA*LF;
                    float j1 = otherLA*otherLF;
                    if (j0 > j1)
                        // Current partial won
                    {
                        // Disconnect for case 1
                        if (mustFight1)
                        {
                            int prevPartialIdx =
                                (*currentPartials)[fight1Idx]._linkedId;
                            if (prevPartialIdx != -1)
                                prevPartials0[prevPartialIdx]._linkedId = -1;
                            
                            (*currentPartials)[fight1Idx]._id = -1;
                            (*currentPartials)[fight1Idx]._linkedId = -1;
                        }
                        
                        currentPartial._id = prevPartial._id;
                        currentPartial._age = prevPartial._age;

                        currentPartial._linkedId = i;
                        prevPartial._linkedId = j;
                        
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

        // Quick optimization
        if (numIters > MAX_NUM_ITER)
            break;
        
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
            
            newPartials.push_back(currentPartial);
        }
    }
    
    // NOTE: sometimes would need an "infinite number of iterations"..
    
    // Add the remaining partials
    remainingCurrentPartials->clear();
    for (int i = 0; i < currentPartials->size(); i++)
    {
        const Partial &p = (*currentPartials)[i];
        if (p.mId == -1)
            remainingCurrentPartials->push_back(p);
    }
    
    // Update current partials
    *currentPartials = newPartials;
}

long
PartialFilterAMFM::findNearestFreqId(const vector<Partial> &partials,
                                     float freq, int index)
{
    if (index > partials.size() - 1)
        index = partials.size() - 1; 
    
    if (partials[index]._freq < freq)
    {
        for (int i = index; i < partials.size(); i++)
        {
            if (partials[i]._freq > freq)
            {
                float d20 = partials[i]._freq - freq;
                float d21 = freq - partials[i - 1]._freq;
                
                if (d20 < d21)
                    return i;
                else
                    return (i - 1);
            }
        }
    }
    else if (partials[index]._freq > freq)
    {
        for (int i = index; i >= 0; i--)
        {
            if (partials[i]._freq < freq)
            {
                float d20 = freq - partials[i]._freq;
                float d21 = partials[i + 1]._freq - freq;
                
                if (d20 < d21)
                    return i;
                else
                    return (i + 1);
            }
        }
    }

    // Index corresponds exactly to the lookup frequency
    return index;
}

void
PartialFilterAMFM::
associatePartialsNeri(const vector<Partial> &prevPartials,
                      vector<Partial> *currentPartials,
                      vector<Partial> *remainingCurrentPartials)
{
    // Parameters
    //float delta = 0.2;
    float delta = _neriDelta;
    float zetaF = 50.0; // in Hz
    float zetaA = 15; // in dB

    zetaF *= 1.0/(_sampleRate*0.5);
    
    // These can't be <= 0
    if (zetaF < 1e-1)
        zetaF = 1e-1;
    if (zetaA < 1e-1)
        zetaA = 1e-1;
 
    //Convert to log from dB
    zetaA = zetaA/20*log(10);

    // Avoid infinite loop
#define MAX_NUM_ITER 10
    
    // Sort current partials and prev partials by increasing frequency
    sort(currentPartials->begin(), currentPartials->end(), Partial::freqLess);
    
    vector<Partial> &prevPartials0 = _tmpPartials5;
    prevPartials0 = prevPartials;

    sort(prevPartials0.begin(), prevPartials0.end(), Partial::idLess);
    
    // Associated partials
    bool stopFlag = true;
    int numIters = 0;
    do {
        stopFlag = true;

        numIters++;
        
        for (int i = 0; i < prevPartials0.size(); i++)
        {
            const Partial &prevPartial = prevPartials0[i];
            
            // Check if the link is already done
            if (((int)prevPartial.mId != -1) &&
                (findPartialById(*currentPartials, (int)prevPartial._id) != -1))
                // Already linked
                continue;
            
            for (int j = 0; j < currentPartials->size(); j++)
            {
                Partial &currentPartial = (*currentPartials)[j];
                
                if (currentPartial.mId == prevPartial._id)
                    continue;

                // Compute current score
                float A;
                float B;
                computeCostNeri(prevPartial, currentPartial,
                                delta, zetaF, zetaA,
                                &A, &B);

                float cost = MIN(A, B);
                
                bool discard = false;
                discard = checkDiscardBigJump(prevPartial, currentPartial);

#if DISCARD_OPPOSITE_DIRECTION
                discard = checkDiscardOppositeDirection(prevPartial, currentPartial);
#endif
                
                // Avoid big jumps or similar
                if (!discard)
                    // Associate!
                {
                    // Current partial already has an id
                    bool mustFight0 = (currentPartial._id != -1);

                    int fight1Idx = findPartialById(*currentPartials,
                                                    (int)prevPartial._id);
                    // Prev partial already has some association with the current id
                    bool mustFight1 = (fight1Idx != -1);
                        
                    if (!mustFight0 && !mustFight1)
                    {
                        currentPartial._id = prevPartial._id;
                        currentPartial._age = prevPartial._age;
                        
                        stopFlag = false;
                        
                        continue;
                    }
                        
                    // Fight!
                    //
                    
                    // Find the previous link for case 0
                    int otherPrevIdx =
                        findPartialByIdSorted(prevPartials0, currentPartial);
                    
                    // Find prev partial
                    Partial prevPartialFight =
                        mustFight0 ? prevPartials0[otherPrevIdx] : prevPartial;
                    // Find current partial
                    Partial currentPartialFight =
                        mustFight0 ? currentPartial : (*currentPartials)[fight1Idx];

                    // Compute other score
                    float otherA;
                    float otherB;
                    computeCostNeri(prevPartialFight, currentPartialFight,
                                    delta, zetaF, zetaA,
                                    &otherA, &otherB);
                    
                    float otherCost = MIN(otherA, otherB);
                    
                    if (cost < otherCost)
                        // Current partial won
                    {
                        currentPartial._id = prevPartial._id;
                        currentPartial._age = prevPartial._age;

                        // Disconnect for case 1
                        if (mustFight1)
                            (*currentPartials)[fight1Idx]._id = -1;
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

        // Quick optimization
        if (numIters > MAX_NUM_ITER)
            break;
        
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
            
            newPartials.push_back(currentPartial);
        }
    }

    // NOTE: sometimes would need an "infinite number of iterations"..
    
    // Add the remaining partials
    remainingCurrentPartials->clear();
    for (int i = 0; i < currentPartials->size(); i++)
    {
        const Partial &p = (*currentPartials)[i];
        if (p._id == -1)
            remainingCurrentPartials->push_back(p);
    }
    
    // Update current partials
    *currentPartials = newPartials;
}

void
PartialFilterAMFM::computeZombieDeadPartials(const vector<Partial> &prevPartials,
                                             const vector<Partial> &currentPartials,
                                             vector<Partial> *zombieDeadPartials)
{
    zombieDeadPartials->clear();
    
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

                if (newPartial._zombieAge < MAX_ZOMBIE_AGE)
                    zombieDeadPartials->push_back(newPartial);
            }
            else if (prevPartial._state == Partial::ZOMBIE)
            {
                Partial newPartial = prevPartial;
                newPartial._zombieAge++;
                
                if (newPartial._zombieAge >= MAX_ZOMBIE_AGE)
                {
                    newPartial._state = Partial::DEAD;
                }
                
                zombieDeadPartials->push_back(newPartial);
            }
            
            // If DEAD, do not add, forget it
        }
    }
}

// Simple fix for partial crossing error
void
PartialFilterAMFM::fixPartialsCrossing(const vector<Partial> &partials0,
                                       const vector<Partial> &partials1,
                                       vector<Partial> *partials2)
{
#if HARD_OPTIM
    return;
#endif
    
    // Optimization
#define MIN_PARTIAL_AGE 5

#define MAX_SWAP_FREQ 100.0/(_sampleRate*0.5)
    
    const vector<Partial> partials2Copy = *partials2;
        
    Partial p0[3];
    Partial p1[3];
    
    for (int i = 0; i < partials2Copy.size(); i++)
    {
        p0[2] = partials2Copy[i];
        if (p0[2]._id == -1)
            continue;

        // Tmp optim
        if (p0[2]._age < MIN_PARTIAL_AGE)
            continue;

        int idx01 = findPartialByIdSorted(partials1, p0[2]);
        
        if (idx01 == -1)
            continue;
        p0[1] = partials1[idx01];

        int idx00 = findPartialByIdSorted(partials0, p0[2]);
        
        if (idx00 == -1)
            continue;
        p0[0] = partials0[idx00];

        //
        for (int j = i + 1; j < partials2Copy.size(); j++)
        {
            p1[2] = partials2Copy[j];
            if (p1[2]._id == -1)
                continue;

            // Try to avoid very messy results
            if ((p1[2]._freq - p0[2]._freq > MAX_SWAP_FREQ) ||
                (p0[2]._freq - p1[2]._freq > MAX_SWAP_FREQ))
                continue;
 
            int idx11 = findPartialByIdSorted(partials1, p1[2]);
            
            if (idx11 == -1)
                continue;
            p1[1] = partials1[idx11];

            int idx10 = findPartialByIdSorted(partials0, p1[2]);
            
            if (idx10 == -1)
                continue;
            p1[0] = partials0[idx10];

            // Extrapolated values
            float extraP0 = p0[1]._freq + (p0[1]._freq - p0[0]._freq);
            float extraP1 = p1[1]._freq + (p1[1]._freq - p1[0]._freq);

            // Check if extrapolated points intersect
            float extraSeg0[2][2] = { { p0[1]._freq, 0.0 }, { extraP0, 1.0 } };
            float extraSeg1[2][2] = { { p1[1]._freq, 0.0 }, { extraP1, 1.0 } };
            bool extraIntersect = Utils::segSegIntersect(extraSeg0, extraSeg1);

            // Check if real points intersect
            float seg0[2][2] = { { p0[1]._freq, 0.0 }, { p0[2]._freq, 1.0 } };
            float seg1[2][2] = { { p1[1]._freq, 0.0 }, { p1[2]._freq, 1.0 } };
            bool intersect = Utils::segSegIntersect(seg0, seg1);
            
            if (intersect != extraIntersect)
            {   
                int tmpId = p0[2]._id;
                p0[2]._id = p1[2]._id;
                p1[2]._id = tmpId;
                
                (*partials2)[i]._id = p0[2]._id;
                (*partials2)[j]._id = p1[2]._id;

                // ??
                break;
            }
        }
    }
}

int
PartialFilterAMFM::findPartialById(const vector<Partial> &partials, int idx)
{
    long numPartials = partials.size();
    for (int i = 0; i < numPartials; i++)
    {
        const Partial &partial = partials[i];
        
        if (partial._id == idx)
            return i;
    }
    
    return -1;
}

int
PartialFilterAMFM::findPartialByIdSorted(const vector<Partial> &partials,
                                         const Partial &refPartial)
{
    vector<Partial> &partials0 = (vector<Partial> &)partials;
    
    // Find the corresponding prev partial
    vector<Partial>::iterator it =
        lower_bound(partials0.begin(), partials0.end(), refPartial, Partial::idLess);
    
    if (it != partials0.end() && (*it).mId == refPartial._id)
    {
        // We found the element!
        return (it - partials0.begin());
    }

    // Not found
    return -1;
}

// Compute amplitude likelihood
// (increase when the penality decrease)
float
PartialFilterAMFM::computeLA(const Partial &prevPartial,
                             const Partial &currentPartial)
{
    float a =
        fabs(prevPartial._amp - (currentPartial._amp - currentPartial._alpha0));
    float b =
        fabs(currentPartial._amp - (prevPartial._amp + prevPartial._alpha0));
    float h = 1.0;
    float area = Utils::trapezoidArea(a, b, h);
    
    // u
    float denom = sqrt(currentPartial._amp*prevPartial._amp);
    float ua = 0.0;
    if (denom > NL_EPS)
        ua = area/denom;
    
    // Likelihood
    float LA = 1.0/(1.0 + ua);
    
    return LA;
}

// Compute frequency likelihood
// (increase when the penality decrease)
float
PartialFilterAMFM::computeLF(const Partial &prevPartial,
                             const Partial &currentPartial)
{
    float a =
        fabs(prevPartial._freq - (currentPartial._freq - currentPartial._beta0));
    float b =
        fabs(currentPartial._freq - (prevPartial._freq + prevPartial._beta0));
    float h = 1.0;
    float area = Utils::trapezoidArea(a, b, h);
    
    // u
    float denom = sqrt(currentPartial._freq*prevPartial._freq);
    float uf = 0.0;
    if (denom > NL_EPS)
        uf = area/denom;
    
    // Likelihood
    float LF = 1.0/(1.0 + uf);
        
    return LF;
}

// See: https://github.com/jundsp/Fast-Partial-Tracking
void
PartialFilterAMFM::computeCostNeri(const Partial &prevPartial,
                                   const Partial &currentPartial,
                                   float delta, float zetaF, float zetaA,
                                   float *A, float *B)
{
    // Set a very big value for zetaA
    zetaA = 50.0; // => this way detections begin to be good
    
    float deltaF =
        (currentPartial._freq - currentPartial._beta0*0.5) -
        (prevPartial._freq + prevPartial._beta0*0.5);
    float deltaA = (currentPartial._amp - currentPartial._alpha0*0.5) -
        (prevPartial._amp + prevPartial._alpha0*0.5);
    

    // Modification from the paper to avoid log of negative values
    float coeff = log((delta - 1.0)/(delta - 2.0));
    float sigmaF2 = -zetaF*zetaF*coeff;
    float sigmaA2 = -zetaA*zetaA*coeff;

    *A = 1.0 - exp(-deltaF*deltaF/sigmaF2 - deltaA*deltaA/sigmaA2);
    
    *B = 1.0 - (1.0 - delta)*(*A);
}   

bool
PartialFilterAMFM::checkDiscardBigJump(const Partial &prevPartial,
                                       const Partial &currentPartial)
{
#define BIG_JUMP_COEFF 16.0

    float oneBinEps = 1.0/_bufferSize;

    // Check if partials are very close
    // (in this case, it sould keep the same id, even if beta0 is very small)
    if (fabs(prevPartial._freq - currentPartial._freq) <
        oneBinEps*BIG_JUMP_COEFF)
        return false;
        
    // Extrapoled frequency, from prev partial
    float extraFreq0 = prevPartial._freq + prevPartial._beta0;
    bool flag0 = (currentPartial._freq > extraFreq0 +
                  BIG_JUMP_COEFF*(extraFreq0 - prevPartial._freq));


    bool flag1 = (currentPartial._freq < extraFreq0 -
                  BIG_JUMP_COEFF*(extraFreq0 - prevPartial._freq));

    // Extrapolated, from current partial
    float extraFreq1 = currentPartial._freq - currentPartial._beta0;
    bool flag2 = (prevPartial._freq > extraFreq1 +
                  BIG_JUMP_COEFF*(extraFreq1 - currentPartial._freq));

    bool flag3 = (prevPartial._freq < extraFreq1 -
                  BIG_JUMP_COEFF*(extraFreq1 - currentPartial._freq));

    // Use flags "&&" to mix cases
    // e.g to give a chance to a case where prev beta0 is almost 0,
    // but current beta0 has a significant value.
    if (flag0 && flag3)
        return true;
    
    if (flag1 && flag2)
        return true;
    
    return false;
}

bool
PartialFilterAMFM::checkDiscardOppositeDirection(const Partial &prevPartial,
                                                 const Partial &currentPartial)
{
    if ((prevPartial._freq < currentPartial._freq) &&
        (prevPartial._beta0 < 0.0) && (currentPartial._beta0 > 0.0))
        return true;
    if ((prevPartial._freq > currentPartial._freq) &&
        (prevPartial._beta0 > 0.0) && (currentPartial._beta0 < 0.0))
        return true;

    if ((prevPartial._amp < currentPartial._amp) &&
        (prevPartial._alpha0 < 0.0) && (currentPartial._alpha0 > 0.0))
        return true;
    if ((prevPartial._amp > currentPartial._amp) &&
        (prevPartial._alpha0 > 0.0) && (currentPartial._alpha0 < 0.0))
        return true;
    
    return false;
}
