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
    // Handle context closing
    nvgDeleteGL2(_nvgContext);
}
   
void
OpenGLNanoVGComponent::drawNanoVGGraphics()
{
    // Example NanoVG drawing: a rectangle and text
    nvgBeginPath(_nvgContext);
    nvgRect(_nvgContext, 50, 50, 200, 100);
    nvgFillColor(_nvgContext, nvgRGBA(255, 255, 0, 0));
    nvgFill(_nvgContext);
    
    nvgFontSize(_nvgContext, 24.0f);
    nvgFontFace(_nvgContext, "Roboto-Bold");
    nvgFillColor(_nvgContext, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(_nvgContext, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(_nvgContext, 150, 100, "NanoVG (GL2)", nullptr);
}
