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
#include "STNUtils.h"
#include "STNProcessorStep0.h"

STNProcessorStep0::STNProcessorStep0(int bufferSize, int overlap, float sampleRate)
{
    _bufferSize = bufferSize;
    _overlap = overlap;    
    _sampleRate = sampleRate;
}

STNProcessorStep0::~STNProcessorStep0() {}

void
STNProcessorStep0::reset()
{
    reset(_bufferSize, _overlap, _sampleRate);
}

void
STNProcessorStep0::reset(int bufferSize, int overlap, float sampleRate)
{
    _bufferSize = bufferSize;
    _overlap = overlap;
    _sampleRate = sampleRate;

    _sinesBuffer.clear();
}

void
STNProcessorStep0::processFFT(const vector<complex<float> > &inBuffer,
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
    STNUtils::decSTN(Rt, 0.7, 0.8, &S, &T, &N);

    // Sines
    vector<complex<float> > xs;
    xs.resize(S.size());
    for (int i = 0; i < xs.size(); i++)
        xs[i] = S[i] * _X[_X.size()/2][i];

    // Rest
    vector<complex<float> > xres;
    xres.resize(T.size());
    for (int i = 0; i < xres.size(); i++)
        xres[i] = (T[i] + N[i]) * _X[_X.size()/2][i];

    // Set outputs
    outBuffers->resize(2);
    (*outBuffers)[0] = xs;
    (*outBuffers)[1] = xres;

    // Fill sines magns buffer
    vector<float> sinesPhases;
    Utils::complexToMagnPhase(&_sinesBuffer, &sinesPhases, xs);
}


int
STNProcessorStep0::getLatency()
{
    int nMedianH;
    int nMedianV;
    STNUtils::computeNMedian(_bufferSize, _overlap, _sampleRate, &nMedianH, &nMedianV);
    
    int latency = (nMedianH / 2)*_bufferSize;

    return latency;
}

void
STNProcessorStep0::getSinesBuffer(vector<float> *buf)
{
    *buf = _sinesBuffer;
}
