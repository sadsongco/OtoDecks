/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include "LevelMeter.h"
#include "OtoDecksLookAndFeel.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public juce::AudioAppComponent,
                        public juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    // implement AudioAppComponent pure virtual functions */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    // implement Component pure virtual functions */
    void paint (Graphics& g) override;
    void resized() override;
    
    // implement Timer pure virtual functions */
    /**
     MainComponent::timerCallback()
     Input                  none
     Output                 none
     Retrieves peak values from MainComponent::leftPeak / rightPeak
     and updates output meters accordingly
     */
    void timerCallback() override;

    
private:
    //==============================================================================
    // Your private member variables go here...
    /** app look and feel */
    OtoDecksLookAndFeel otoDecksLookAndFeel;
    /** colour palette */
    juce::Colour controllerBackground, controllerBody, controllerIndicator, infoTextColour, warningTextColour;
    /** stores and manages the available audio formats */
    juce::AudioFormatManager formatManager;
    /** stores the thumbnail for waveform display */
    juce::AudioThumbnailCache thumbcache{100};

    // instantiate two players and their respective GUIs */
    DJAudioPlayer player1{formatManager};
    DeckGUI* deckGUI1 = new DeckGUI{&player1, formatManager, thumbcache, 0};

    DJAudioPlayer player2{formatManager};
    DeckGUI* deckGUI2 = new DeckGUI{&player2, formatManager, thumbcache, 1};
    
    /** mixersource for audio output */
    juce::MixerAudioSource mixerSource;
    
    /** playlist component */
    PlaylistComponent playlistComponent{formatManager, player1, deckGUI1, player2, deckGUI2};
    
    /** padding around edge of app window */
    int drawGutter = 20;
    
    // instantiate left and right level meters for the audio output */
    LevelMeter outputLevelL{255, false};
    LevelMeter outputLevelR{255, false};
    
    /** local variable storing peak value of buffer */
    float leftPeak = 0;
    /** local variable storing peak value of buffer */
    float rightPeak = 0;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
