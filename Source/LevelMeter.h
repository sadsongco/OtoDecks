/*
  ==============================================================================

    LevelMeter.h
    Created: 12 Aug 2020 3:16:16pm
    Author:  Nigel Powell

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class LevelMeter  : public juce::Component
{
public:
    LevelMeter(
               int _meterResolution,
               bool _vertical
               );
    ~LevelMeter() override;

    // implement component virtual methods
    void paint (juce::Graphics&) override;
    void resized() override;
    void initialise(int _meterResolution, int _peakFalloff, float _meterFalloff, bool _vertical);
    
    /* ===================== */
    /* ====== methods ====== */
    /* ===================== */
    
    /**
     LevelMeter::displayLevel()
     Input                  double
     Output                 none
     @param level           a double in the range 0 - 1 to be shown as an audio level
     Takes the passed level, updates the currentSmoothedLevel
     Calculates and stores numBlocksToPaint and finalBlockAlpha
     Calls repaint() on the component
     */
    void displayLevel(double level);
    
private:
    /* ======================== */
    /* ====== properties ====== */
    /* ======================== */
    
    /** controls how fast currentSmoothedLevel decreases */
    float meterFalloff;
    /** the maximum number of meter blocks */
    int meterResolution;
    /* the size of each meter block to be painted */
    double meterPaintBlockSize;
    /* the number of meter blocks that will be painted */
    int numBlocksToPaint;
    /* stores current peak for peak hold **not yet implemented** */
    double currentPeak;
    /** controls how fast the peak hold decreases **not yet implemented** */
    int peakFalloff;
    /** a smoothed value for the input level to the meter */
    double currentSmoothedLevel;
    /** value for alpha value of final block depending on level */
    float finalBlockAlpha;
    /** control thresholds for colour and silence */
    float meterOrangeTransition, meterRedTransition, meterMinLevelThreshold;
    /** meter paint colours */
    juce::Colour meterGreen{0xff00a100}, meterOrange{0xffe3e805}, meterRed{0xffbd0404};
    // to do - figure out why this can't be initialised with hex values in constructor
    /** if true meter is drawn vertically, otherwise drawn horizontally */
    bool vertical;
  
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
