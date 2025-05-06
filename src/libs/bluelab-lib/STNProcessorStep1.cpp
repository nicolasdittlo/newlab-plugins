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

#include "Defines.h"
#include "Utils.h"
#include "STNProcessorStep1.h"

STNProcessorStep1::STNProcessorStep1(int bufferSize, int overlap, float sampleRate)
{
    _bufferSize = bufferSize;
    _overlap = overlap;    
    _sampleRate = sampleRate;
}

STNProcessorStep1::~STNProcessorStep1() {}

void
STNProcessorStep1::reset()
{
    reset(_bufferSize, _overlap, _sampleRate);
}

void
STNProcessorStep1::reset(int bufferSize, int overlap, float sampleRate)
{
    _bufferSize = bufferSize;
    _overlap = overlap;
    _sampleRate = sampleRate;

    _noiseBuffer.clear();
}

void
STNProcessorStep1::processFFT(const vector<complex<float> > &inBuffer,
                              vector<vector<complex<float> > > *outBuffers)
{    
    vector<float> magns;
    vector<float> phases;
    Utils::complexToMagnPhase(&magns, &phases, inBuffer);

    int nMedianH;
    int nMedianV;
    STNUtils::computeNMedian(_bufferSize, _overlap, _sampleRate, &nMedianH, &nMedianV);

    _X.push_front(inBuffer);
    if (_X.size() > nMedianH)
        _X.pop_back();
    
    _XMagn.push_front(magns);
    if (_XMagn.size() > nMedianH)
        _XMagn.pop_back();
    
    vector<float> Rt;
    STNUtils::transientness(_XMagn, nMedianH, nMedianV, &Rt);

    vector<float> S;
    vector<float> T;
    vector<float> N;
    STNUtils::decSTN(Rt, 0.75, 0.85);

    // Transients
    vector<complex<float> > xt;
    xt.resize(T.size());
    for (int i = 0; i < xt.size(); i++)
        xt[i] = T[i] * _X[_X.size()/2][i];

    // Noise
    vector<complex<float> > xn;
    xn.resize(T.size());
    for (int i = 0; i < xn.size(); i++)
        xn[i] = (S[i] + N[i]) * _X[_X.size()/2][i];

    // Fill outputs
    outBuffers->resize(2);
    (*outBuffers)[0] = xt;
    (*outBuffers)[1] = xn;

    // Noise magns buffer
    vector<float> noisePhases;
    Utils::complexToMagnPhase(&_noiseBuffer, &noisePhases, xn);
}


int
STNProcessorStep1::getLatency()
{
    int nMedianH;
    int nMedianV;
    STNUtils::computeNMedian(_bufferSize, _overlap, _sampleRate, &nMedianH, &nMedianV);
    
    int latency = (nMedianH / 2)*_bufferSize;

    return latency;
}

void
STNProcessorStep1::getNoiseBuffer(vector<float> *buf)
{
    *buf = _noiseBuffer;
}
