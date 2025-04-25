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
    _sampleRate = sampleRate;
}

void
STNProcessorStep0::processFFT(vector<complex<float> > *ioBuffer)
{    
    vector<float> magns;
    vector<float> phases;
    Utils::complexToMagnPhase(&magns, &phases, *ioBuffer);

    // TODO
}


int
STNProcessorStep0::getLatency()
{
    // TODO
    
    return 0;
}
