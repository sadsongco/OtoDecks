/*
  ==============================================================================

    otoDecksLookAndFeel.h
    Created: 22 Aug 2020 1:25:18pm
    Author:  Nigel Powell

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class OtoDecksLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OtoDecksLookAndFeel();
    ~OtoDecksLookAndFeel();
    /**
     OtoDecksLookAndFeel::setColourPalette()
     input                  juce::Colour variables
     output                 none
     sets the colours to the passed juce::Colour variables so the entire app colour scheme can be set in one place
     */
    void setColourPalette(juce::Colour& _controllerBackground,
                          juce::Colour& _controllerBody,
                          juce::Colour& _controllerIndicator,
                          juce::Colour& _infoTextColour,
                          juce::Colour& _warningTextColour);
private:
    // override LookAndFeel virtual methods
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override;
    void drawLinearSlider    (juce::Graphics& g,
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
    void drawLabel (juce::Graphics& g, Label& label) override;
    void drawButtonText (juce::Graphics& g,
                         juce::TextButton& button,
                         bool shouldDrawHighlighted,
                         bool shouldDrawDown) override;
    void drawButtonBackground (juce::Graphics& g,
                               Button& button,
                               const Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown
                               ) override;
    void fillTextEditorBackground (Graphics& g,
                                   int width,
                                   int height,
                                   TextEditor& textEditor
                                   ) override;
    void drawTextEditorOutline (Graphics& g,
                                int width,
                                int height,
                                TextEditor& textEditor
                                ) override;
    
    void drawCustomButton (Graphics& g,
                           float x, float y, float width, float height,
                           const Colour& colour, float outlineThickness, float cornerSize
                           );
    
    // set colour palette options
    juce::Colour controllerBackground, controllerBody, controllerIndicator, infoTextColour, warningTextColour;
};
