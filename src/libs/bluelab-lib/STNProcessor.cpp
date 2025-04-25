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

#include <stdlib.h>

#include "Utils.h"
#include "OverlapAdd.h"
#include "STNProcessorStep0.h"
#include "STNProcessor.h"

#define OVERLAP_STEP0 8

#define FFT_SIZE_COEFF_STEP0 5 // fft size: 8192

STNProcessor::STNProcessor()
{
    _overlapAddStep0 = NULL;
    _processorStep0 = NULL;
}

STNProcessor::~STNProcessor()
{
    if (_overlapAddStep0 != NULL)
        delete _overlapAddStep0;

    if (_processorStep0 != NULL)
        delete _processorStep0;
}

void
STNProcessor::prepareToPlay(double sampleRate)
{
    int fftSizeStep0 = Utils::nearestPowerOfTwo(sampleRate/FFT_SIZE_COEFF_STEP0);

    if (_overlapAddStep0 == NULL)
        _overlapAddStep0 = new OverlapAdd(fftSizeStep0, OVERLAP_STEP0, true, true);

    _overlapAddStep0->setFftSize(fftSizeStep0);
    _overlapAddStep0->setOverlap(OVERLAP_STEP0);
    
    if (_processorStep0 == NULL)
        _processorStep0 = new STNProcessorStep0(fftSizeStep0, OVERLAP_STEP0, sampleRate);

    _processorStep0->reset(fftSizeStep0, OVERLAP_STEP0, sampleRate);

    _overlapAddStep0->addProcessor(_processorStep0);
}

int
STNProcessor::getLatency()
{
    // TODO
    return 0;
}

void
STNProcessor::setSinesMix(float mix)
{
    _sinesMix = mix;
}

void
STNProcessor::setTransientsMix(float mix)
{
    _transientsMix = mix;
}

void
STNProcessor::setNoiseMix(float mix)
{
    _noiseMix = mix;
}

void
STNProcessor::setMuteSines(bool mute)
{
    _muteSines = mute;
}

void
STNProcessor::setMuteTransients(bool mute)
{
    _muteTransients = mute;
}

void
STNProcessor::setMuteNoise(bool mute)
{
    _muteNoise = mute;
}
    
void
STNProcessor::process(const vector<float> input, vector<float> *output)
{
    _overlapAddStep0->feed(input);

    int numSamplesToFlush = _overlapAddStep0->getOutSamples(output, input.size());
    _overlapAddStep0->flushOutSamples(numSamplesToFlush);

    // TODO
}

void
STNProcessor::getSinesBuffer(vector<float> *buf)
{
    // TODO
}

void
STNProcessor::getNoiseBuffer(vector<float> *buf)
{
    // TODO
}
