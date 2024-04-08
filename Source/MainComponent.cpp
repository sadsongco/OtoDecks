/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // initialise look and feel for the app
    controllerBackground = juce::Colours::teal;
    controllerBody = juce::Colours::maroon;
    controllerIndicator = juce::Colours::goldenrod;
    infoTextColour = juce::Colours::antiquewhite;
    warningTextColour = juce::Colours::dimgrey;
    otoDecksLookAndFeel.setColourPalette(controllerBackground,
                                         controllerBody,
                                         controllerIndicator,
                                         infoTextColour,
                                         warningTextColour);
    setLookAndFeel(&otoDecksLookAndFeel);
    deckGUI1->setColourPalette(controllerBackground,
                               controllerBody,
                               controllerIndicator,
                               infoTextColour,
                               warningTextColour);
    deckGUI2->setColourPalette(controllerBackground,
                               controllerBody,
                               controllerIndicator,
                               infoTextColour,
                               warningTextColour);
    playlistComponent.setColourPalette(controllerBackground,
                               controllerBody,
                               controllerIndicator,
                               infoTextColour,
                               warningTextColour);

    // initialise main component and child component variables
    setSize (900, 600);

    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (2, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }
    
    outputLevelL.setName("ouput level L");
    outputLevelR.setName("output level R");

    addAndMakeVisible(playlistComponent);
    addAndMakeVisible(outputLevelL);
    addAndMakeVisible(outputLevelR);

    formatManager.registerBasicFormats();
    
    startTimer(50);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    stopTimer();
    delete deckGUI1;
    delete deckGUI2;
    setLookAndFeel(nullptr);
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);
 }

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);
    // store peak levels for current buffer if larger than stored peak
    float currLPeak = bufferToFill.buffer->getMagnitude(0, 0, bufferToFill.numSamples);
    float currRPeak = bufferToFill.buffer->getMagnitude(1, 0, bufferToFill.numSamples);
    if (currLPeak > leftPeak)
        leftPeak = currLPeak;
    if (currRPeak > rightPeak)
        rightPeak = currRPeak;
}

void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();
    mixerSource.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setFont(20.0f);
}

void MainComponent::resized()
{
    playlistComponent.setBounds(drawGutter, drawGutter + 30, getWidth() - drawGutter * 2, getHeight() - 30 - drawGutter * 2);
    outputLevelL.setBounds(drawGutter, drawGutter + 2, getWidth() - (drawGutter * 2), 10);
    outputLevelR.setBounds(drawGutter, drawGutter + 13, getWidth() - (drawGutter * 2), 10);
}

// timer called every 50ms
void MainComponent::timerCallback()
{
    // updates the meter output
    if (playlistComponent.isPlaying())
    {
        outputLevelL.displayLevel(leftPeak);
        outputLevelR.displayLevel(rightPeak);
        leftPeak = 0;
        rightPeak = 0;
    }
    else
    {
        // feed zeroes to meters so they can fade back to nothing
        outputLevelL.displayLevel(0);
        outputLevelR.displayLevel(0);
    }
}

