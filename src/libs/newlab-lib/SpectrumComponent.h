#pragma once

#include <stdlib.h>

#include "OpenGLNanoVGComponent.h"
#include "SpectrumView.h"

class SpectrumComponent : public OpenGLNanoVGComponent
{
 public:
    SpectrumComponent()
    {
        _spectrumView = NULL;
    }
    
    ~SpectrumComponent() {}

    void setSpectrumView(SpectrumView *spectrumView)
    {
        _spectrumView = spectrumView;
    }
    
    void drawNanoVGGraphics()
    {
        _spectrumView->draw(_nvgContext);
    }

    void resized() override
    {
        _spectrumView->setViewSize(getWidth(), getHeight());
    }
    
 protected:
    SpectrumView *_spectrumView;
};
