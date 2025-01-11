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

#ifndef AWEIGHTING_H
#define AWEIGHTING_H


// See: https://en.wikipedia.org/wiki/A-weighting
//
// and: https://community.plm.automation.siemens.com/t5/Testing-Knowledge-Base/What-is-A-weighting/ta-p/357894
//

class AWeighting
{
public:
    // numBins is fftSize/2 !
    // See: http://support.ircam.fr/docs/AudioSculpt/3.0/co/FFT%20Size.html
    //
    static void computeAWeights(vector<float> *result,
                                int numBins, float sampleRate);
    
    static float computeAWeight(int binNum, int numBins, float sampleRate);
    
protected:
    static float ComputeR(float frequency);
    
    static float ComputeA(float frequency);
};

#endif
