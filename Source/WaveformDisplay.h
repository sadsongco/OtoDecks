/*
  ==============================================================================

    WaveformDisplay.h
    Created: 4 Jul 2020 1:22:39pm
    Author:  Nigel Powell

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class WaveformDisplay  :    public juce::LookAndFeel_V4
{
public:
    WaveformDisplay(
                    juce::AudioFormatManager &formatManagerToUse,
                    juce::AudioThumbnailCache &cacheToUse
                    );
    ~WaveformDisplay() override;

    // juce LookAndFeel virtual methods
    void drawLinearSlider (juce::Graphics& g,
                              int     x,
                              int     y,
                              int     width,
                              int     height,
                              float     sliderPos,
                              float     minSliderPos,
                              float     maxSliderPos,
                              const juce::Slider::SliderStyle,
                              juce::Slider &
                              ) override;
    void drawLabel (juce::Graphics& g,
                    juce::Label& label) override;
    
    /**
     WaveformDisplay::setColourPalette()
     input                  juce::Colour variables
     output                 none
     sets the colours used in this component's paint() method
     to the passed juce::Colour variables so the entire app colour scheme can be set in one place
     */
    void setColourPalette(juce::Colour& _controllerBackground,
                          juce::Colour& _controllerBody,
                          juce::Colour& _controllerIndicator,
                          juce::Colour& _infoTextColour,
                          juce::Colour& _warningTextColour);
    
    /**
     WaveformDisplay::loadURL()
     input                  juce::URL
     output                 none
     loads in a new audio thumbnail for the passed URL
     */
    void loadURL(URL audioURL);
    
private:
    
    /** colour palette */
    juce::Colour controllerBackground, controllerBody, controllerIndicator, infoTextColour, warningTextColour;
    // thumbnail of waveform
    juce::AudioThumbnail audioThumb;
    // flag for if a file is loaded
    bool fileLoaded;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
