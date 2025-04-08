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

#include <JuceHeader.h>

#include <nanovg.h>

class OpenGLNanoVGComponent : public juce::Component, private juce::OpenGLRenderer
{
public:
    OpenGLNanoVGComponent();
    ~OpenGLNanoVGComponent() override;
    
    // OpenGLRenderer callbacks
    void newOpenGLContextCreated() override;

    void renderOpenGL() override;
    
    void openGLContextClosing() override;
    
protected:
    NVGcontext* _nvgContext = nullptr;
    
    virtual void drawNanoVGGraphics();
    
private:
    void checkOpenGLVersion();
    
    std::unique_ptr<juce::OpenGLContext> _openGLContext;

    bool _openGLVersionValid;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLNanoVGComponent)
};
