/*
  ==============================================================================

    otoDecksLookAndFeel.cpp
    Created: 22 Aug 2020 1:25:18pm
    Author:  Nigel Powell

  ==============================================================================
*/

#include "OtoDecksLookAndFeel.h"
#include <iostream>

OtoDecksLookAndFeel::OtoDecksLookAndFeel()
{
    
}
OtoDecksLookAndFeel::~OtoDecksLookAndFeel()
{
    
}

void OtoDecksLookAndFeel::setColourPalette(juce::Colour &_controllerBackground,
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

void OtoDecksLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                       const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&)
{
    auto radius = (float) juce::jmin (width / 3, height / 3) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    // outline
    g.setColour (controllerBody);
    g.drawEllipse (rx, ry, rw, rw, 2.0f);
    // pointer
    juce::Path p;
    auto pointerLength = radius * 0.9f;
    auto pointerThickness = 3.0f;
    p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (controllerIndicator);
    g.fillPath (p);
    // endstops
    p.clear();
    p.startNewSubPath(0, -radius);
    p.lineTo(0, -radius + 3.0f);
    p.closeSubPath();
    juce::Path p2{p};
    p.applyTransform(juce::AffineTransform::rotation(rotaryStartAngle).translated(centreX, centreY));
    g.setColour(juce::Colours::darkgrey);
    g.strokePath(p, juce::PathStrokeType(1.0f));
    p2.applyTransform(juce::AffineTransform::rotation(rotaryEndAngle).translated(centreX, centreY));
    g.strokePath(p2, juce::PathStrokeType(1.0f));
}

void OtoDecksLookAndFeel::drawLinearSlider    (juce::Graphics& g,
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
    if (slider.getName() == "crossfade" || slider.getName() == "crossfade time")
    {
        slider.getName() == "crossfade" ? g.setFont(height) : g.setFont(width * 0.16f);
        g.setColour(controllerBackground.withAlpha(0.4f));
        g.drawText(slider.getName(), x, y, width, height, juce::Justification::centred, false);
    }
    if (slider.getName() == "crossfade")
    {
        juce::Path p;
        g.setColour(controllerIndicator);
        if (int(sliderPos) >= width - 1)
            sliderPos -= 2;
        g.fillRect((int)sliderPos, y, 2, height);
    }
    else
    {
        g.setColour(juce::Colours::maroon.withAlpha(0.5f));
        if (int(sliderPos) >= width - 1)
            sliderPos -= 2;
        g.fillRect(x + 2, y + 2, (int)sliderPos, height - 4);
    }
}

void OtoDecksLookAndFeel::drawLabel (juce::Graphics& g, Label& label)
{
    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));
        
        if (label.getAttachedComponent() == nullptr)
            g.setColour(warningTextColour.withMultipliedAlpha(alpha));
        else
            g.setColour (infoTextColour.withMultipliedAlpha(alpha));
        
        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());
        
        if (label.getParentComponent()->getName() != "crossfade")
            g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                              jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                              label.getMinimumHorizontalScale());
        else
        {
            auto indent = label.getWidth() / 10;
            std::vector<std::string> tokens;
            std::istringstream iss(label.getText().toStdString());
            std::string token;
            while (std::getline(iss, token, '\t'))
            {
                tokens.push_back(token);
            }
            g.drawText(tokens[0], indent, 0, label.getWidth() - (indent * 2), label.getHeight(), juce::Justification::centredLeft);
            g.drawText(tokens[1], indent, 0, label.getWidth() - (indent * 2), label.getHeight(), juce::Justification::centredRight);
        }
        
        g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour (controllerBody);
    }
    if (label.getAttachedComponent() == nullptr)
        g.setColour(controllerBody);
    else
        g.setColour (juce::Colours::transparentWhite);
    
    g.drawRect (label.getLocalBounds());
}

void OtoDecksLookAndFeel::drawButtonText (juce::Graphics& g,
                     juce::TextButton& button,
                     bool shouldDrawHighlighted,
                     bool shouldDrawDown)
{
    Font font (getTextButtonFont (button, button.getHeight()));
    font.setHeight(font.getHeight() * 0.8f);
    g.setFont(font);
    if (button.getName() == "auto play")
        button.getToggleStateValue() == 1 ? g.setColour(controllerIndicator) : g.setColour (warningTextColour);
    else
        g.setColour(controllerIndicator);
    
    const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;
    
    const int fontHeight = roundToInt (font.getHeight() * 0.6f);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;
    
    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          Justification::centred, 2);
}

void OtoDecksLookAndFeel::drawButtonBackground (juce::Graphics& g,
                           Button& button,
                           const Colour& backgroundColour,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown
                           )
{
    const int width = button.getWidth();
    const int height = button.getHeight();
    
    const float outlineThickness = button.getToggleStateValue() == 1 ? ((shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted) ? 1.2f : 0.7f) : 0.4f;
    
    juce::Colour baseColour;
    if (button.getName() == "auto play")
    {
        button.getToggleStateValue() == 1 ? baseColour = controllerBackground : baseColour = controllerBackground.withAlpha(0.5f);
    }
    else
        baseColour = controllerBackground;
    
    drawCustomButton (g,
                      0,
                      0,
                      (float) width,
                      (float) height,
                      baseColour, outlineThickness, -1.0f);
}

void OtoDecksLookAndFeel::fillTextEditorBackground (Graphics& g,
                               int width,
                               int height,
                               TextEditor& textEditor
                               )
{
    g.setColour(juce::Colours::grey.withAlpha(0.2f));
    g.fillAll();
    if (!textEditor.hasKeyboardFocus(true))
    {
        g.setColour(warningTextColour);
        g.drawText(textEditor.getTextToShowWhenEmpty(), 0, 0, width, height, juce::Justification::centred);
    }
}
void OtoDecksLookAndFeel::drawTextEditorOutline (Graphics& g,
                            int width,
                            int height,
                            TextEditor& textEditor
                            )
{
    g.setColour(controllerBody);
    g.drawLine(width, 0, width, height);
}

void OtoDecksLookAndFeel::drawCustomButton (Graphics& g,
                       float x, float y, float width, float height,
                       const Colour& colour, float outlineThickness, float cornerSize)
{
    // simplified from juce::LookAndFeel_V2::drawGlassLozenge()
    if (width <= outlineThickness || height <= outlineThickness)
        return;
    
    auto cs = cornerSize < 0 ? jmin (width * 0.5f, height * 0.5f) : cornerSize;
    auto edgeBlurRadius = height * 0.75f + (height - cs * 2.0f);
    
    auto indentW = width / 16;
    auto indentH = height / 16;
    
    Path outline;
    outline.addRoundedRectangle (x + indentW, y + indentH, width - (2 * indentW), height - (2 * indentH), cs);
    
    {
        ColourGradient cg (colour.darker (0.2f), 0, y,
                           colour.darker (0.2f), 0, y + height, false);
        
        cg.addColour (0.03, colour.withMultipliedAlpha (0.3f));
        cg.addColour (0.4, colour);
        cg.addColour (0.97, colour.withMultipliedAlpha (0.3f));
        
        g.setGradientFill (cg);
        g.fillPath (outline);
        g.setColour(controllerBody);
        g.strokePath(outline, juce::PathStrokeType(2.0f));
    }
    
    ColourGradient cg (Colours::transparentBlack, x + edgeBlurRadius, y + height * 0.5f,
                       colour.darker (0.2f), x, y + height * 0.5f, true);
    
    cg.addColour (jlimit (0.0, 1.0, 1.0 - (cs * 0.5f) / edgeBlurRadius), Colours::transparentBlack);
    cg.addColour (jlimit (0.0, 1.0, 1.0 - (cs * 0.25f) / edgeBlurRadius), colour.darker (0.2f).withMultipliedAlpha (0.3f));
}
