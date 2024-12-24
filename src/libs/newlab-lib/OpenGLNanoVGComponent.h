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
    juce::OpenGLContext _openGLContext;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLNanoVGComponent)
};
