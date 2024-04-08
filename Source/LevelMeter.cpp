/*
  ==============================================================================

    LevelMeter.cpp
    Created: 12 Aug 2020 3:16:16pm
    Author:  Nigel Powell

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LevelMeter.h"

//==============================================================================
LevelMeter::LevelMeter(
                       int _meterResolution,
                       bool _vertical
                       ) :
                        meterResolution(_meterResolution),
                        vertical(_vertical)
{
    // initialise properties
    meterPaintBlockSize = 0;
    numBlocksToPaint = 0;
    currentPeak = 0;
    currentSmoothedLevel = 0;
    finalBlockAlpha = 1.0f;
    meterOrangeTransition = 0.6f;
    meterRedTransition = 0.93f;
    meterMinLevelThreshold = 0.01f;
    meterFalloff = 0.95;
}

LevelMeter::~LevelMeter()
{
    
}

void LevelMeter::paint (juce::Graphics& g)
{
    int fullScale;
    if (!vertical)
        fullScale = getWidth() / meterPaintBlockSize;
    else
        fullScale = getHeight() / meterPaintBlockSize;
    for (int i = 0; i < numBlocksToPaint; ++i)
    {
        if (i > fullScale * meterRedTransition)
            g.setColour(meterRed);
        else if (i > fullScale * meterOrangeTransition)
            g.setColour(meterOrange);
        else
            g.setColour(meterGreen);
        
        if (!vertical)
            g.drawLine(meterPaintBlockSize * i, 0, meterPaintBlockSize * i, getHeight(), 1);
        else
            g.drawLine(0, getHeight() - (meterPaintBlockSize * i), getWidth(), getHeight() - (meterPaintBlockSize * i), 1);
    }
    // final block
    if (numBlocksToPaint > fullScale * meterRedTransition)
        g.setColour(meterRed.withAlpha(finalBlockAlpha));
    else if (numBlocksToPaint > fullScale * meterOrangeTransition)
        g.setColour(meterOrange.withAlpha(finalBlockAlpha));
    else
        g.setColour(meterGreen.withAlpha(finalBlockAlpha));
    if (!vertical)
        g.drawLine(meterPaintBlockSize * numBlocksToPaint, 0, meterPaintBlockSize * numBlocksToPaint, getHeight(), 1);
    else
        g.drawLine(0, getHeight() - (meterPaintBlockSize * numBlocksToPaint), getWidth(), getHeight() - (meterPaintBlockSize * numBlocksToPaint), 1);

    g.setColour (juce::Colours::white);
    g.setFont (10.0f);
    g.drawText (getName(), getLocalBounds(),
                juce::Justification::left, true);   // draw some placeholder text
}

void LevelMeter::resized()
{
    if (!vertical)
        meterPaintBlockSize = getWidth() / meterResolution;
    else
        meterPaintBlockSize = getHeight() / meterResolution;
}

void LevelMeter::displayLevel(double level)
{
    if (level > currentSmoothedLevel)
    {
        currentSmoothedLevel = level;
    }
    else
    {
        // faster falloff for lower levels
        if (currentSmoothedLevel < 0.2)
            meterFalloff = 0.8;
        currentSmoothedLevel *= meterFalloff;
        if (level > currentSmoothedLevel)
            currentSmoothedLevel = level;
        if (currentSmoothedLevel < meterMinLevelThreshold)
            currentSmoothedLevel = 0;
    }
    if (level > currentPeak)
        currentPeak = level;
    if (!vertical)
    {
        numBlocksToPaint = ((int)(getWidth()*currentSmoothedLevel)/meterPaintBlockSize);
        finalBlockAlpha = 1.0f * (((getWidth() * currentSmoothedLevel) - (numBlocksToPaint * meterPaintBlockSize)) / meterPaintBlockSize);
    }
    else
    {
        numBlocksToPaint = ((int)(getHeight()*currentSmoothedLevel)/meterPaintBlockSize);
        finalBlockAlpha = 1.0f * (((getHeight() * currentSmoothedLevel) - (numBlocksToPaint * meterPaintBlockSize)) / meterPaintBlockSize);
    }

    repaint();
}
