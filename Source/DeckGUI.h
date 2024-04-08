/*
  ==============================================================================

    DeckGUI.h
    Created: 27 Jun 2020 11:00:50am
    Author:  Nigel Powell

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"
#include "LevelMeter.h"
#include "OtoDecksLookAndFeel.h"


//==============================================================================
/*
*/
class DeckGUI    :  public juce::Component,
                    public juce::Button::Listener,
                    public juce::Slider::Listener,
                    public juce::ChangeBroadcaster,
                    public juce::ChangeListener,
                    public juce::FileDragAndDropTarget,
                    public juce::Timer
{
public:
    DeckGUI(DJAudioPlayer* _player,
            juce::AudioFormatManager &formatManagerToUse,
            juce::AudioThumbnailCache &cacheToUse,
            short int _guiID);
    ~DeckGUI();

    // implement Component pure virtual methods */
    void paint (Graphics&) override;
    void resized() override;
 
    /* ======================== */
    /* ====== properties ====== */
    /* ======================== */
    
    /** flags for end of track */
    bool streamEnded = false, streamNearlyEnded = false;
    /** flag for whether player has a file loaded */
    bool fileLoaded = false;
    /** display name for loaded audio file */
    juce::String currentTrackName;
    /** store this track's est. bpm, and the bpm in the other player */
    int bpm, targetBpm;
    
    /* ===================== */
    /* ====== methods ====== */
    /* ===================== */
    
    /**
     DeckGUI::loadFile()
     Input                  juce::URL
     Output                 none
     @param url             juce::URL of an audio file to be loaded into the player
     Takes passed URL, loads it into associated player and waveform display
     Initialises player, GUI and DeckGUI flags read by PlaylistComponent
     */
    void loadFile(juce::URL url);
    
    /**
     DeckGUI::play()
     Input                  none
     Output                 none
     If stream has ended it returns the transport to the beginning of the file
     Flips the play state of the associated player
     */
    void play();
    
    /**
     DeckGUI::isPlaying()
     Input                  none
     Output                 bool indicating player status
     Returns true if associated player is playing, false if not
     */
    bool isPlaying();
    
    /**
     DeckGUI::toTrackStart()
     Input                  none
     Output                 none
     Sets associated player transport to beginning of audio file
     */
    void toTrackStart();
    
    /**
     DeckGUI::setCrossfadeRatio()
     Input                  double
     Output                 none
     @param ratio           double between 0 and 1 of the ratio the player should be set to
     Sets associated player crossfade ratio to the value given
     ratio usually passed from PlaylistComponent
     */
    void setCrossfadeRatio(double ratio);
    
    /**
     DeckGUI::setColourPalette()
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

private:
    // implement juce virtual / pure virtual methods */
    
    // implement Button::Listener */
    void buttonClicked(juce::Button *) override;
    
    // implement Slider::Listener */
    void sliderValueChanged(juce::Slider*) override;
    void sliderDragStarted(juce::Slider*) override;
    void sliderDragEnded(juce::Slider*) override;
    
    // implement file drag and drop **/
    bool isInterestedInFileDrag (const juce::StringArray &files) override;
    void filesDropped (const juce::StringArray &files, int x, int y) override;
    
    // implement changeListener */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // implement timer */
    void timerCallback() override;
    
    /* ======================== */
    /* ====== properties ====== */
    /* ======================== */
    
    //identify whether GUI is on left or right
    short int guiID;
    
    /** colour palette for use in this component's paint method */
    juce::Colour controllerBackground, controllerBody, controllerIndicator, infoTextColour, warningTextColour;

    // GUI element components */
    juce::ImageButton playButton{"Play"}, returnButton{"Return"}, loadButton{"Load"}, matchTempoButton{"Match Tempo"};
    juce::Slider volSlider{juce::Slider::Rotary, juce::Slider::TextBoxLeft}, speedSlider{juce::Slider::Rotary, juce::Slider::TextBoxLeft}, posSlider{};
    juce::Label volLabel, speedLabel, posLabel;
    // GUI presentation control
    juce::Path playIcon, pauseIcon, returnIcon, loadIcon, matchTempoIcon;
    
    /** stores current crossfade ratio */
    double currentCrossfadeRatio;
    
    /** pointer to DJAudioPlayer which the GUI will interact with */
    DJAudioPlayer* player;
    
    /** object to display audio waveform thumbnail */
    WaveformDisplay waveformDisplay;
    
    /** level meters for left and right output of associated player */
    LevelMeter levelL{31, true}, levelR{31, true};
    
    /** class variables for grid layout */
    double rowH, colW;
    float padding;
    /** class variables for meter layout */
    double meterWidth, meterMargin, meterBetween;
    // these above don't need class scope, but hey ho, I can afford a handful of wasted bytes
    
    /** stores the current status of the player: no file loaded, play, stop, queued */
    juce::String playerStatus;
    /* ===================== */
    /* ====== methods ====== */
    /* ===================== */
    
    /**
     DeckGUI::openFileChooser()
     Input                  none
     Output                 none
     Opens a juce file chooser dialog, passes File to DeckGUI::loadFile()
     */
    void openFileChooser();
    /**
     DeckGUI::matchTempo()
     Input                  none
     Output                 none
     Calculates a ratio of DeckGUI::bpm to DeckGUI::targetBpm,
     and changes the value of DeckGUI::speedSlider accordingly
     */
    void matchTempo();
    /** DeckGUI::secondsToMinutesAndSeconds()
     Input                  double seconds
     Output                 juce::String minutes(') seconds(")
     Calculates minutes and seconds from a passed value of seconds and returns a display string
     */
    juce::String secondsToMinutesAndSeconds(double value);
    
    // component virtual method - can start / stop focussed DeckGUI with space bar
    bool keyPressed(const KeyPress &key) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckGUI)
};
