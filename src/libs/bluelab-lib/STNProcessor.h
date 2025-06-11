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

/* Implementation of
   Enhanced Fuzzy Decomposition of Sound Into Sines, Transients and Noise
   Leonardo Fierro and Vesa Välimäki
*/

#ifndef STN_PROCESSOR_H
#define STN_PROCESSOR_H

#include <vector>
using namespace std;

class OverlapAdd;
class MultiOutOverlapAdd;
class STNProcessorStep0;
class STNProcessorStep1;
class Delay;

class STNProcessor
{
 public:
    STNProcessor();

    virtual ~STNProcessor();

    void prepareToPlay(double sampleRate);

    int getLatency();
        
    void setSinesMix(float mix);
    void setTransientsMix(float mix);
    void setNoiseMix(float mix);

    void setMuteSines(bool mute);
    void setMuteTransients(bool mute);
    void setMuteNoise(bool mute);
    
    void process(const vector<float> input, vector<float> *output);

    // time/amplitude
    void getSinesBuffer(vector<float> *buf);
    void getNoiseBuffer(vector<float> *buf);
    
 protected:
    MultiOutOverlapAdd *_overlapAddStep0;
    STNProcessorStep0 *_processorStep0;

    MultiOutOverlapAdd *_overlapAddStep1;
    STNProcessorStep1 *_processorStep1;

    // Used to delay the step 0 sines signal
    Delay * _step1Delay;
    
    float _sinesMix;
    float _transientsMix;
    float _noiseMix;

    bool _muteSines;
    bool _muteTransients;
    bool _muteNoise;

    vector<float> _sinesBuffer;
    vector<float> _noiseBuffer;
};

#endif
