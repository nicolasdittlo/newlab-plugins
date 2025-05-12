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
#include "Delay.h"
#include "OverlapAdd.h"
#include "MultiOutOverlapAdd.h"
#include "STNProcessorStep0.h"
#include "STNProcessorStep1.h"
#include "STNProcessor.h"

#define OVERLAP_STEP0 4 //8 // 4
#define OVERLAP_STEP1 4 //8 // 4

#define FFT_SIZE_COEFF_STEP0 5 //6 //5 //6 // fft size: 8192 at 44100Hz
#define FFT_SIZE_COEFF_STEP1 86 //43 //86 //43 // fft size: 512 at 44100Hz

STNProcessor::STNProcessor()
{
    _overlapAddStep0 = NULL;
    _processorStep0 = NULL;

    _overlapAddStep1 = NULL;
    _processorStep1 = NULL;

    _step1Delay = NULL;
}

STNProcessor::~STNProcessor()
{
    if (_overlapAddStep0 != NULL)
        delete _overlapAddStep0;

    if (_processorStep0 != NULL)
        delete _processorStep0;

    if (_overlapAddStep1 != NULL)
        delete _overlapAddStep1;

    if (_processorStep1 != NULL)
        delete _processorStep1;

    if (_step1Delay != NULL)
        delete _step1Delay;
}

void
STNProcessor::prepareToPlay(double sampleRate)
{
    // Step 0
    int fftSizeStep0 = Utils::nearestPowerOfTwo(sampleRate/FFT_SIZE_COEFF_STEP0);

    if (_overlapAddStep0 == NULL)
        _overlapAddStep0 = new MultiOutOverlapAdd(fftSizeStep0, OVERLAP_STEP0, 2, true, true);

    _overlapAddStep0->setFftSize(fftSizeStep0);
    _overlapAddStep0->setOverlap(OVERLAP_STEP0);
    
    if (_processorStep0 == NULL)
    {
        _processorStep0 = new STNProcessorStep0(fftSizeStep0, OVERLAP_STEP0, sampleRate);
        _overlapAddStep0->addProcessor(_processorStep0);
    }

    _processorStep0->reset(fftSizeStep0, OVERLAP_STEP0, sampleRate);

    // Step 1
    int fftSizeStep1 = Utils::nearestPowerOfTwo(sampleRate/FFT_SIZE_COEFF_STEP1);

    if (_overlapAddStep1 == NULL)
        _overlapAddStep1 = new MultiOutOverlapAdd(fftSizeStep1, OVERLAP_STEP1, 2, true, true);

    _overlapAddStep1->setFftSize(fftSizeStep1);
    _overlapAddStep1->setOverlap(OVERLAP_STEP1);
    
    if (_processorStep1 == NULL)
    {
        _processorStep1 = new STNProcessorStep1(fftSizeStep1, OVERLAP_STEP1, sampleRate);
        _overlapAddStep1->addProcessor(_processorStep1);
    }
    
    _processorStep1->reset(fftSizeStep1, OVERLAP_STEP0, sampleRate);

    // Delay
    int hopSizeStep1 = fftSizeStep1/OVERLAP_STEP1;        
    int step1Latency = (fftSizeStep1 - hopSizeStep1) + _processorStep1->getLatency();
    if (_step1Delay == NULL)
        _step1Delay = new Delay(step1Latency);
    _step1Delay->reset();
    _step1Delay->setDelay(step1Latency);
}

int
STNProcessor::getLatency(int blockSize)
{
    int latency = 0;

    if (_overlapAddStep0 != NULL)
    {
        int fftSize = _overlapAddStep0->getFftSize();
        int hopSize = fftSize/_overlapAddStep0->getOverlap();
        
        latency += fftSize - hopSize;

        if (blockSize < hopSize)
            latency += hopSize - blockSize;
    }
    
    if (_processorStep0 != NULL)
        latency += _processorStep0->getLatency();

    if (_overlapAddStep1 != NULL)
    {
        int fftSize = _overlapAddStep1->getFftSize();
        int hopSize = fftSize/_overlapAddStep1->getOverlap();
        
        latency += fftSize - hopSize;
    }
    
    if (_processorStep1 != NULL)
        latency += _processorStep1->getLatency();

    return latency;
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
    // Step 0
    _overlapAddStep0->feed(input);

    vector<vector<float> > samplesStep0;
    samplesStep0.resize(2);
    int numSamplesToFlush0 = _overlapAddStep0->getOutSamples(&samplesStep0, input.size());
    _overlapAddStep0->flushOutSamples(numSamplesToFlush0);

    vector<float> &xs = samplesStep0[0];
    vector<float> &xres = samplesStep0[1];

    // Step 1
    _overlapAddStep1->feed(xres);

    vector<vector<float> > samplesStep1;
    samplesStep1.resize(2);
    int numSamplesToFlush1 = _overlapAddStep1->getOutSamples(&samplesStep1, input.size());
    _overlapAddStep1->flushOutSamples(numSamplesToFlush1);

    vector<float> &xt = samplesStep1[0];
    vector<float> &xn = samplesStep1[1];

    // Step 1 delay
    if (_step1Delay != NULL)
        _step1Delay->processSamples(&xs);

    // Apply gains
    Utils::multValue(&xs, _sinesMix);
    if (_muteSines)
        Utils::multValue(&xs, 0.0);

    Utils::multValue(&xt, _transientsMix);
    if (_muteTransients)
        Utils::multValue(&xt, 0.0);

    Utils::multValue(&xn, _noiseMix);
    if (_muteNoise)
        Utils::multValue(&xn, 0.0);

    // Sum
    output->resize(input.size());
    Utils::fillZero(output);
    for (int i = 0; i < output->size(); i++)
    {
        (*output)[i] += xs[i];
        (*output)[i] += xt[i];
        (*output)[i] += xn[i];
    }
}

void
STNProcessor::getSinesBuffer(vector<float> *buf)
{
    buf->clear();
    if (_processorStep0 != NULL)
    {
        _processorStep0->getSinesBuffer(buf);

        Utils::multValue(buf, _sinesMix);
        if (_muteSines)
            Utils::multValue(buf, 0.0);
    }
}

void
STNProcessor::getNoiseBuffer(vector<float> *buf)
{
    buf->clear();
    if (_processorStep1 != NULL)
    {
        _processorStep1->getNoiseBuffer(buf);

        Utils::multValue(buf, _noiseMix);
        if (_muteNoise)
            Utils::multValue(buf, 0.0);
    }
}
