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

#include <JuceHeader.h>
#include <GL/gl.h>

using namespace juce::gl;

#include <nanovg.h>

#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg_gl.h>

#include "OpenGLNanoVGComponent.h"

// Use MSAA 4x gives better antialiasing results than nanovg
#define USE_MSAA 1

OpenGLNanoVGComponent::OpenGLNanoVGComponent()
{
    // Configure the OpenGL pixel format with a stencil buffer
    juce::OpenGLPixelFormat pixelFormat;
    pixelFormat.stencilBufferBits = 8; // Request an 8-bit stencil buffer

#if USE_MSAA
    pixelFormat.multisamplingLevel = 4; // Use 4x MSAA
#endif
    _openGLContext.setPixelFormat(pixelFormat);
        
    _openGLContext.setRenderer(this);
    _openGLContext.attachTo(*this);
    //_openGLContext.setContinuousRepainting(true); // Optional, for continuous updates
}

OpenGLNanoVGComponent::~OpenGLNanoVGComponent()
{
    if (_nvgContext)
        nvgDeleteGL2(_nvgContext);
    
    _openGLContext.detach();
}

// OpenGLRenderer callbacks
void
OpenGLNanoVGComponent::newOpenGLContextCreated()
{
    juce::gl::loadFunctions();
    juce::gl::loadExtensions();
    
#if !USE_MSAA
    _nvgContext = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
#else
    // Check MSAA support
    GLint samples = 0;
    GLint sampleBuffers = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);

    if (samples >= 4)
        // Use MSAA antialiasing
        _nvgContext = nvgCreateGL2(NVG_STENCIL_STROKES);
    else
        // Fallback, use nanovg antialiasing
        _nvgContext = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
#endif
    jassert(_nvgContext != nullptr);

    // Load font from memory
    int fontIndex = nvgCreateFontMem(_nvgContext,
                                     "Roboto-Bold",  // Internal name used by NanoVG
                                     (unsigned char *)BinaryData::RobotoBold_ttf,  // Pointer to the font data
                                     BinaryData::RobotoBold_ttfSize,  // Size of the font data
                                     0  // Do not free the memory (managed by JUCE)
                                     );

    if (fontIndex == -1)
        DBG("Failed to load font Roboto-Bold");
}

void
OpenGLNanoVGComponent::renderOpenGL()
{    
    // Clear the screen
    juce::OpenGLHelpers::clear(juce::Colours::black);
        
    // Get component size
    const int width = getWidth();
    const int height = getHeight();
    
    // Set NanoVG viewport
    glViewport(0, 0, width, height);
    nvgBeginFrame(_nvgContext, width, height, static_cast<float>(width) / height);
    
    // Draw using NanoVG
    drawNanoVGGraphics();
    
    nvgEndFrame(_nvgContext);
}

void
OpenGLNanoVGComponent::openGLContextClosing()
{
    if (_nvgContext)
    {
        nvgDeleteGL2(_nvgContext);
        _nvgContext = nullptr;
    }
}
   
void
OpenGLNanoVGComponent::drawNanoVGGraphics()
{
    // Example NanoVG drawing: a rectangle and text
    nvgBeginPath(_nvgContext);
    nvgRect(_nvgContext, 50, 50, 200, 100);
    nvgFillColor(_nvgContext, nvgRGBA(255, 0, 0, 255));
    nvgFill(_nvgContext);
    
    nvgFontSize(_nvgContext, 24.0f);
    nvgFontFace(_nvgContext, "Roboto-Bold");
    nvgFillColor(_nvgContext, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(_nvgContext, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(_nvgContext, 150, 100, "NanoVG (GL2)", nullptr);
}
