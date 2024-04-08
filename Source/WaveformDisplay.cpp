/*
  ==============================================================================

    WaveformDisplay.cpp
    Created: 4 Jul 2020 1:22:39pm
    Author:  Nigel Powell

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveformDisplay.h"

//==============================================================================
WaveformDisplay::WaveformDisplay(
    AudioFormatManager &formatManagerToUse,
    AudioThumbnailCache &cacheToUse
    ) :
    audioThumb(1000, formatManagerToUse, cacheToUse)
{
}

WaveformDisplay::~WaveformDisplay()
{
}

void WaveformDisplay::setColourPalette(juce::Colour &_controllerBackground,
                               juce::Colour &_controllerBody,
                               juce::Colour &_controllerIndicator,
                               juce::Colour &_infoTextColour,
                               juce::Colour &_warningTextColour)
{
    controllerBackground = _controllerBackground;
    controllerBody = _controllerBody;
    controllerIndicator = _controllerIndicator;
    infoTextColour = _infoTextColour;
    warningTextColour = _warningTextColour;
}

void WaveformDisplay::drawLinearSlider    (juce::Graphics& g,
                                               int     x,
                                               int     y,
                                               int     width,
                                               int     height,
                                               float     sliderPos,
                                               float     minSliderPos,
                                               float     maxSliderPos,
                                               const juce::Slider::SliderStyle style,
                                               juce::Slider& slider
                                               )
{
    juce::Rectangle<int> sliderRect{x, y, width, height};
    g.setColour(controllerBackground);
    g.fillRect(sliderRect);
    
    g.setColour (controllerBody);
    g.drawRect (sliderRect);   // draw an outline around the component
    
    if (fileLoaded)
    {
        audioThumb.drawChannels(
                               g,
                               sliderRect,
                               0,
                               audioThumb.getTotalLength(),
                                1.0f
                               );
    }
    else
    {
        g.setColour(warningTextColour);
        slider.hideTextBox(true);
        g.setFont (20.0);
        g.drawText ("File not loaded...", sliderRect,
                    juce::Justification::centred, true);   // draw some placeholder text
    }
    g.setColour(juce::Colours::goldenrod);
    g.fillRect((int)sliderPos, y, 2, height);
}

void WaveformDisplay::drawLabel (juce::Graphics& g,
                juce::Label& label)
{
    // override, position as minutes and seconds will be shown elsewhere in the GUI
}

void WaveformDisplay::loadURL(URL audioURL)
{
    audioThumb.clear();
    fileLoaded = audioThumb.setSource(new URLInputSource(audioURL));
}
