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

#include "OpenGLNanoVGComponent.h"
#include "SpectrumViewNVG.h"

class SpectrumComponentGL : public OpenGLNanoVGComponent
{
 public:
    SpectrumComponentGL()
    {
        _spectrumView = NULL;
    }
    
    ~SpectrumComponentGL() {}

    void setSpectrumView(SpectrumViewNVG *spectrumView)
    {
        _spectrumView = spectrumView;
    }
    
    void drawNanoVGGraphics() override
    {
        _spectrumView->draw(_nvgContext);
    }

    void resized() override
    {
        _spectrumView->setViewSize(getWidth(), getHeight());
    }
    
 protected:
    SpectrumViewNVG *_spectrumView;
};
