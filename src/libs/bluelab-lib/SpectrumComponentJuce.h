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

#pragma once

#include <stdlib.h>

#include "SpectrumViewJuce.h"

class SpectrumComponentJuce : public juce::Component
{
 public:
    SpectrumComponentJuce()
    {
        _spectrumView = NULL;
    }
    
    ~SpectrumComponentJuce() {}

    void setSpectrumView(SpectrumViewJuce *spectrumView)
    {
        _spectrumView = spectrumView;
    }
    
    void paint(juce::Graphics &g) override
    {
        _spectrumView->paint(g);
    }

    void resized() override
    {
        _spectrumView->setViewSize(getWidth(), getHeight());
    }
    
 protected:
    SpectrumViewJuce *_spectrumView;
};
