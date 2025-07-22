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
#include "ParamSmoother.h"
#include "OverlapAdd.h"
#include "MultiOutOverlapAdd.h"
#include "STNProcessorStep0.h"
#include "STNProcessorStep1.h"
#include "STNProcessor.h"

#define OVERLAP_STEP0 4 //8
#define OVERLAP_STEP1 4 //8

#define FFT_SIZE_COEFF_STEP0 5 //6 // fft size for 5: 8192 at 44100Hz
#define FFT_SIZE_COEFF_STEP1 86 //43 // fft size for 86: 512 at 44100Hz

// Enable or disable transients extraction
#define EXTRACT_TRANSIENTS 1 //0

STNProcessor::STNProcessor()
{
    _overlapAddStep0 = NULL;
    _processorStep0 = NULL;

    _overlapAddStep1 = NULL;
    _processorStep1 = NULL;

    _step1Delay = NULL;

    _sinesMixSmoother = NULL;
    _transientsMixSmoother = NULL;
    _noiseMixSmoother = NULL;

    _sinesMix = 1.0;
    _transientsMix = 1.0;
    _noiseMix = 1.0;

    _muteSines = false;
    _muteTransients = false;
    _muteNoise = false;
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

    if (_sinesMixSmoother != NULL)
        delete _sinesMixSmoother;

    if (_transientsMixSmoother != NULL)
        delete _transientsMixSmoother;

    if (_noiseMixSmoother != NULL)
        delete _noiseMixSmoother;
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
    
    _processorStep1->reset(fftSizeStep1, OVERLAP_STEP1, sampleRate);

    // Delay
    int step1Latency = fftSizeStep1 + _processorStep1->getLatency();
    
    if (_step1Delay == NULL)
        _step1Delay = new Delay(step1Latency);
    _step1Delay->reset();
    _step1Delay->setDelay(step1Latency);

    if (_sinesMixSmoother == NULL)
        _sinesMixSmoother = new ParamSmoother(sampleRate, _sinesMix);
    _sinesMixSmoother->resetToTargetValue(_sinesMix);
    _sinesMixSmoother->reset(sampleRate);

    if (_transientsMixSmoother == NULL)
        _transientsMixSmoother = new ParamSmoother(sampleRate, _transientsMix);
    _transientsMixSmoother->resetToTargetValue(_transientsMix);
    _transientsMixSmoother->reset(sampleRate);

    if (_noiseMixSmoother == NULL)
        _noiseMixSmoother = new ParamSmoother(sampleRate, _noiseMix);
    _noiseMixSmoother->resetToTargetValue(_noiseMix);
    _noiseMixSmoother->reset(sampleRate);
}

int
STNProcessor::getLatency()
{
    int latency = 0;

    if (_overlapAddStep0 != NULL)
    {
        int fftSize = _overlapAddStep0->getFftSize();
        
        latency += fftSize;
    }
    
    if (_processorStep0 != NULL)
        latency += _processorStep0->getLatency();

    if (_overlapAddStep1 != NULL)
    {
        int fftSize = _overlapAddStep1->getFftSize();
        
        latency += fftSize;
    }
    
    if (_processorStep1 != NULL)
        latency += _processorStep1->getLatency();
    
    return latency;
}

void
STNProcessor::setSinesMix(float mix)
{
    _sinesMix = mix;

    if (_sinesMixSmoother != NULL)
        _sinesMixSmoother->setTargetValue(_sinesMix);
}

void
STNProcessor::setTransientsMix(float mix)
{
    _transientsMix = mix;

    if (_transientsMixSmoother != NULL)
        _transientsMixSmoother->setTargetValue(_transientsMix);
}

void
STNProcessor::setNoiseMix(float mix)
{
    _noiseMix = mix;

    if (_noiseMixSmoother != NULL)
        _noiseMixSmoother->setTargetValue(_noiseMix);
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

#if EXTRACT_TRANSIENTS
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
#else
    vector<float> &xn = xres;
#endif
    
    // Apply gains
    if (_muteSines)
        Utils::multValue(&xs, 0.0);
    else
    {
        if (_sinesMixSmoother != NULL)
            Utils::applyGain(xs, &xs, _sinesMixSmoother);
    }
    
#if EXTRACT_TRANSIENTS
    if (_muteTransients)
        Utils::multValue(&xt, 0.0);
    else
    {
        if (_transientsMixSmoother != NULL)
            Utils::applyGain(xt, &xt, _transientsMixSmoother);
    }
#endif
    
    if (_muteNoise)
        Utils::multValue(&xn, 0.0);
    else
    {
        if (_noiseMixSmoother != NULL)
            Utils::applyGain(xn, &xn, _noiseMixSmoother);
    }
    
    // Sum
    output->resize(input.size());
    for (int i = 0; i < output->size(); i++)
    {
#if EXTRACT_TRANSIENTS
        (*output)[i] = xs[i] + xt[i] + xn[i];
#else
        (*output)[i] = xs[i] + xn[i];
#endif
    }

    // buffers
    _sinesBuffer = xs;
    _noiseBuffer = xn;
}

void
STNProcessor::getSinesBuffer(vector<float> *buf)
{
    *buf = _sinesBuffer;
}

void
STNProcessor::getNoiseBuffer(vector<float> *buf)
{
    *buf = _noiseBuffer;
}
