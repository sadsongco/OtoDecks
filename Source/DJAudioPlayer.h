/*
  ==============================================================================

    DJAudioPlayer.h
    Created: 23 Jun 2020 10:13:11am
    Author:  Nigel Powell

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>
#include "BPMCalculator.h"

class DJAudioPlayer :
    public juce::AudioSource,
    public juce::ChangeListener,
    public juce::ChangeBroadcaster
{
public:
    DJAudioPlayer(AudioFormatManager& _formatManager);
    ~DJAudioPlayer();
    // implement juce::AudioSource virtual functions
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
 
    /* ======================== */
    /* ====== properties ====== */
    /* ======================== */
    
    /** flag set by DeckGUI while scrubbing to trigger lowering of volume */
    bool dimWhileScrubbing = false;
    
    /** float of bpm for current playing track */
    float currentBPM;

    /* ===================== */
    /* ====== methods ====== */
    /* ===================== */
    
    /**
     DJAudioPlayer::loadURL
     Input                  juce::URL
     Output                 none
     @param audioURL        juce::URL passed from a DeckGUI for loading into the player
     Takes a resource locator, checks if file pointed to is valid audio
     If it is it creates a unique pointer to the file and sets it as the transport source of the player
     */
    void loadURL(URL audioURL);
    
    /**
     DJAudioPlayer::getGain()
     Input                  none
     Output                 double
     Returns the current stored gain of the player
     */
    double getGain();
    
    /**
     DJAudioPlayer::setGain()
     Input                  double gain
     Output                 none
     @param gain            double passed from DeckGUI to set gain
     Takes an input double and sets the current gain of the player to that value
     */
    void setGain(double gain);
    
    /**
     DJAudioPlayer::setCrossfadeRatio()
     Input                  double ratio
     Output                 none
     @param ratio           double passed from PlaylistComponent to set crossfade between players
     Takes an input double and sets the current crossfade ratio of the player to that value
     */
    void setCrossfadeRatio(double ratio);
    
    /**
     DJAudioPlayer::setSpeed()
     Input                  double ratio
     Output                 none
     @param ratio           double passed from DeckGUI to set speed ratio
     Takes an input double and sets the current speed ratio of the player to that value
     */
    void setSpeed(double ratio);
    
    /**
     DJAudioPlayer::setPosition()
     Input                  double posInSeconds
     Output                 none
     @param posInSeconds    double passed from DJAudioPlayer::setPositionRelative to set position
     Takes an input double and sets the current absolute transport position to that value
     */
    void setPosition(double posInSeconds);
    
    /**
     DJAudioPlayer::setPositionRelative()
     Input                  double posInSeconds
     Output                 none
     @param posInSeconds    double passed from DeckGUI to set position
     Takes an input double, calculates the absolute position from the passed
     relative position, and passes that to DJAudioPlayer::setPosition()
     */
    void setPositionRelative(double posInSeconds);
    
    /**
     DJAudioPlayer::getLengthInSeconds()
     Input                  none
     Output                 int of rounded audio length
     Queries the transport source for the length of the current audio file in seconds and returns it
     */
    int getLengthInSeconds();
    
    /**
     DJAudioPlayer::getLevels()
     Input                  none
     Output                 vector<double> of left and right levels from the player
     Returns the current stored peak levels to a DeckGUI for metering in range 0 - 1
     Resets peak levels to zero
     */
    std::vector<double> getLevels();
    
    /** DJAudioPlayer::isPlaying()
     Input                  none
     Output                 bool of player status
     Returns true if player is playing, false if it is not
     */
    bool isPlaying();
    
    /** DJAudioPlayer::start()
     Input                  none
     Output                 none
     Starts playback of the player's transport source
     */
    void start();
    
    /** DJAudioPlayer::stop()
     Input                  none
     Output                 none
     Halts playback of the player's transport source
     */
    void stop();
    
    /**
     DJAudioPlayer::getPositionRelative()
     Input                  none
     Output                 double of transport position
     Returns position of transport as a decimal of the total length of the current audio file
     */
    double getPositionRelative();
    
    /**
     DJAudioPlayer::getPosition()
     Input                  none
     Output                 double of transport position
     Returns position of transport as an absolute value in seconds
     */
    double getPosition();
    
    /**
     DJAudioPlayer::hasStreamFinished()
     Input                  none
     Output                 bool of player status
     Returns true if transport stream has finished
     */
    bool hasStreamFinished();
    
private:
    // implement changeListener pure virtual method */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    /* ======================== */
    /* ====== properties ====== */
    /* ======================== */
    
    // juce derived
    /** stores and manages the available audio formats */
    juce::AudioFormatManager& formatManager;
    /** points to a resource of audio data */
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    /** controls flow of data from a reader source of audio data */
    juce::AudioTransportSource transportSource;
    /** processes the output data from an audio transport source */
    juce::ResamplingAudioSource resampleSource{&transportSource, false, 2};
    
    // native
    /** store current gain / crossfade levels */
    double currentGain, currentCrossfadeRatio;
    /** store peak levels for left / right */
    float leftPeak = 0, rightPeak = 0;
    /** class to calculate the bpm of the currently playing song */
    BPMCalculator bpmCalculator;
};
