#include <JuceHeader.h>
#include <GL/gl.h>

using namespace juce::gl;

#include <nanovg.h>

#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg_gl.h>

#include "OpenGLNanoVGComponent.h"

OpenGLNanoVGComponent::OpenGLNanoVGComponent()
{
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
    
    _nvgContext = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    jassert(_nvgContext != nullptr);
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
    // Handle context closing if necessary
}
   
void
OpenGLNanoVGComponent::drawNanoVGGraphics()
{
    // Example NanoVG drawing: a rectangle and text
    nvgBeginPath(_nvgContext);
    nvgRect(_nvgContext, 50, 50, 200, 100);
    nvgFillColor(_nvgContext, nvgRGBA(255, 192, 0, 255));
    nvgFill(_nvgContext);
    
    nvgFontSize(_nvgContext, 24.0f);
    nvgFontFace(_nvgContext, "sans");
    nvgFillColor(_nvgContext, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(_nvgContext, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(_nvgContext, 150, 100, "Hello, NanoVG (GL2)!", nullptr);
}
