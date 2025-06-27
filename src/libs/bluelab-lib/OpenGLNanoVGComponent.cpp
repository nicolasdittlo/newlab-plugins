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

#include <JuceHeader.h>
#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

using namespace juce::gl;

#include <nanovg.h>

#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg_gl.h>

#include "OpenGLNanoVGComponent.h"

// Use MSAA 4x gives better antialiasing results than nanovg
#define USE_MSAA 1

OpenGLNanoVGComponent::OpenGLNanoVGComponent()
{
    _openGLVersionValid = true;
    
    // Configure the OpenGL pixel format with a stencil buffer
    juce::OpenGLPixelFormat pixelFormat;
    pixelFormat.stencilBufferBits = 8; // Request an 8-bit stencil buffer

#if USE_MSAA
    pixelFormat.multisamplingLevel = 4; // Use 4x MSAA
#endif
    
    _openGLContext = std::make_unique<juce::OpenGLContext>();
    _openGLContext->setPixelFormat(pixelFormat);
    //_openGLContext->setSwapInterval(0);
    _openGLContext->setRenderer(this);
    _openGLContext->attachTo(*this);
    
#ifdef _WIN32
    _openGLContext->setContinuousRepainting(true); // Optional, for continuous updates
#endif
}

OpenGLNanoVGComponent::~OpenGLNanoVGComponent()
{
    if (_nvgContext)
    {
        if (_openGLVersionValid)
            nvgDeleteGL2(_nvgContext);
        _nvgContext = nullptr;
    }
    
    _openGLContext->detach();
}

// OpenGLRenderer callbacks
void
OpenGLNanoVGComponent::newOpenGLContextCreated()
{
    juce::gl::loadFunctions();
    juce::gl::loadExtensions();

    checkOpenGLVersion();

    if (!_openGLVersionValid)
        return;
    
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
    
    float scale = 1.0;
    
#ifdef __APPLE__
    scale = juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds())->scale;
#endif
    
    // Set NanoVG viewport
    glViewport(0, 0, width*scale, height*scale);

    if (_openGLVersionValid)
    {
        // Don't scale here, so the font and other will have a good scale
        nvgBeginFrame(_nvgContext, width, height, static_cast<float>(width) / height);

        // Draw using NanoVG
        drawNanoVGGraphics();
    
        nvgEndFrame(_nvgContext);
    }
}

void
OpenGLNanoVGComponent::openGLContextClosing()
{
    if (_openGLVersionValid)
    {
        if (_nvgContext)
        {
            nvgDeleteGL2(_nvgContext);
            _nvgContext = nullptr;
        }
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

void
OpenGLNanoVGComponent::checkOpenGLVersion()
{
    int majorVersion = 0;
    
#ifndef __APPLE__
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
#else
    const char* versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    int minorVersion = 0;

    if (versionString) {
        sscanf(versionString, "%d.%d", &majorVersion, &minorVersion);
    }
#endif
    
    _openGLVersionValid = (majorVersion >= 2);

    if (glStencilMask == nullptr)
        _openGLVersionValid = false;
    if (glStencilFunc == nullptr)
        _openGLVersionValid = false;
    if (glBlendFuncSeparate == nullptr)
        _openGLVersionValid = false;
    if (glGetShaderInfoLog == nullptr)
        _openGLVersionValid = false;
    if (glGetProgramInfoLog == nullptr)
        _openGLVersionValid = false;
    if (glCreateProgram == nullptr)
        _openGLVersionValid = false;
    if (glCreateShader == nullptr)
        _openGLVersionValid = false;
    if (glShaderSource == nullptr)
        _openGLVersionValid = false;
    if (glCompileShader == nullptr)
        _openGLVersionValid = false;
    if (glGetShaderiv == nullptr)
        _openGLVersionValid = false;
    if (glCompileShader == nullptr)
        _openGLVersionValid = false;
    if (glAttachShader == nullptr)
        _openGLVersionValid = false;
    if (glBindAttribLocation == nullptr)
        _openGLVersionValid = false;
    if (glLinkProgram == nullptr)
        _openGLVersionValid = false;
    if (glGetProgramiv == nullptr)
        _openGLVersionValid = false;
    if (glDeleteProgram == nullptr)
        _openGLVersionValid = false;
    if (glDeleteShader == nullptr)
        _openGLVersionValid = false;
    if (glGetUniformLocation == nullptr)
        _openGLVersionValid = false;
    if (glGetUniformBlockIndex == nullptr)
        _openGLVersionValid = false;
    if (glGenBuffers == nullptr)
        _openGLVersionValid = false;
    if (glUniform4fv == nullptr)
        _openGLVersionValid = false;
    if (glColorMask == nullptr)
        _openGLVersionValid = false;
    if (glStencilOpSeparate == nullptr)
        _openGLVersionValid = false;
    if (glDrawArrays == nullptr)
        _openGLVersionValid = false;
    if (glStencilOp == nullptr)
        _openGLVersionValid = false;
    if (glUseProgram == nullptr)
        _openGLVersionValid = false;
    if (glActiveTexture == nullptr)
        _openGLVersionValid = false;
    if (glBindBuffer == nullptr)
        _openGLVersionValid = false;
    if (glBufferData == nullptr)
        _openGLVersionValid = false;
    if (glEnableVertexAttribArray == nullptr)
        _openGLVersionValid = false;
    if (glVertexAttribPointer == nullptr)
        _openGLVersionValid = false;
    if (glUniform1i == nullptr)
        _openGLVersionValid = false;
    if (glUniform2fv == nullptr)
        _openGLVersionValid = false;
    if (glDisableVertexAttribArray == nullptr)
        _openGLVersionValid = false;
    if (glDeleteVertexArrays == nullptr)
        _openGLVersionValid = false;
    if (glDeleteBuffers == nullptr)
        _openGLVersionValid = false;
}
