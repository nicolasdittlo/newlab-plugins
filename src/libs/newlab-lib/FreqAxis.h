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

#ifndef FREQ_AXIS_H
#define FREQ_AXIS_H

#include "Scale.h"

class FreqAxis
{
 public:
    FreqAxis(bool displayLines, Scale::Type scale);

    virtual ~FreqAxis();

    void init(Axis *axis,
              int bufferSize, float sampleRate);
    
    void reset(int bufferSize, float sampleRate);
    
    void setMaxFreq(float maxFreq);
    float getMaxFreq() const;

    void setScale(Scale::Type scale);
    
 protected:
    void getMinMaxFreqAxisValues(float *minHzValue, float *maxHzValue,
                                 int bufferSize, float sampleRate);
        
    void update();
    
    void updateAxis(int numAxisData,
                    const float freqs[],
                    const char *labels[],
                    float minHzValue, float maxHzValue);
    
    Axis *_axis;
    
    int _bufferSize;
    float _sampleRate;
    
    bool _displayLines;
    
    Scale::Type _scale;

    float _maxFreq;
};

#endif
