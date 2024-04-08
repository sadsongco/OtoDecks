/*
  ==============================================================================

    DJAudioPlayer.cpp
    Created: 23 Jun 2020 10:13:11am
    Author:  Nigel Powell

  ==============================================================================
*/

#include "DJAudioPlayer.h"
#include <iostream>

DJAudioPlayer::DJAudioPlayer(AudioFormatManager& _formatManager)
    :   formatManager(_formatManager)
{
    transportSource.addChangeListener(this);
}

DJAudioPlayer::~DJAudioPlayer()
{
}

void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void DJAudioPlayer::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // set level as gain without crossfade so GUI meter shows level of track
    transportSource.setGain(currentGain);
    resampleSource.getNextAudioBlock(bufferToFill);
    // get output of player for meters
    float currLMax = bufferToFill.buffer->getMagnitude(0, 0, bufferToFill.numSamples);
    float currRMax = bufferToFill.buffer->getMagnitude(1, 0, bufferToFill.numSamples);
    if (currLMax > leftPeak)
        leftPeak = currLMax;
    if (currRMax > rightPeak)
        rightPeak = currRMax;
    // push samples to bpm calculator block, if full do stuff
    for (int i = 0; i < bufferToFill.numSamples; ++i)
    {
        std::pair<float, float> currentSample{bufferToFill.buffer->getSample(0, i), bufferToFill.buffer->getSample(1, i)};
        if (bpmCalculator.blockBuffer.size() >= bpmCalculator.samplesPerBlock)
        {
            bpmCalculator.getBlockEnergy();
            currentBPM = bpmCalculator._bpm;
        }
        bpmCalculator.blockBuffer.push(currentSample);
    }
    // apply crossfade
    bufferToFill.buffer->applyGain(currentCrossfadeRatio);
    // if scrubbing, reduce volume
    if (dimWhileScrubbing)
        bufferToFill.buffer->applyGain(0.1);
}

void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

void DJAudioPlayer::loadURL(URL audioURL)
{
    auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));
    if (reader != nullptr) // good file
    {
        std::unique_ptr<AudioFormatReaderSource> newSource (new AudioFormatReaderSource(reader, true));
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset (newSource.release());
        // inform bpm calculator of track sample rate, initialise
        bpmCalculator.sampleRate = reader->sampleRate;
        bpmCalculator.localPeakCounter = 0;
        bpmCalculator.localBeatCounter = 0;
        bpmCalculator.beatCounter = 0;
        bpmCalculator.instantBpm.clear();
        bpmCalculator._bpm = -1;
    }
}

double DJAudioPlayer::getGain()
{
    return currentGain;
}

void DJAudioPlayer::setGain(double gain)
{
    if (gain < 0 || gain > 100)
        std::cout << "DJAudioPlayer::setGain gain should be in the range 0 - 100" << std::endl;
    else
    {
//        transportSource.setGain(gain);
        currentGain = gain;
    }
    
}

void DJAudioPlayer::setCrossfadeRatio(double ratio)
{
    if (ratio < 0 || ratio > 1)
        std::cout << "DJAudioPlayer::setCrossfadeRatio should be in the range 0 - 1" << std::endl;
    else
    {
        currentCrossfadeRatio = ratio;
    }
}

void DJAudioPlayer::setSpeed(double ratio)
{
    if (ratio < 0.5 || ratio > 2)
        std::cout << "DJAudioPlayer::setSpeed ratio should be in the range 50 - 200%" << std::endl;
    else
        resampleSource.setResamplingRatio(ratio);
}

void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
    if (pos < 0 || pos > 1.0)
        std::cout << "DJAudioPlayer::setPositionRelative pos should be between 0 and 1" << std::endl;
    else
    {
        double posInSeconds = pos * transportSource.getLengthInSeconds();
        setPosition(posInSeconds);
    }
}

bool DJAudioPlayer::isPlaying()
{
    return transportSource.isPlaying();
}

void DJAudioPlayer::start()
{
    transportSource.start();
}

void DJAudioPlayer::stop()
{
    transportSource.stop();
}

double DJAudioPlayer::getPosition()
{
    return transportSource.getCurrentPosition();
}

double DJAudioPlayer::getPositionRelative()
{
    if (transportSource.getLengthInSeconds() > 0)
        return transportSource.getCurrentPosition() / transportSource.getLengthInSeconds();
    else
        return 0.0;
}

int DJAudioPlayer::getLengthInSeconds()
{
    return transportSource.getLengthInSeconds();
}

std::vector<double> DJAudioPlayer::getLevels()
{
    std::vector<double> levels(2);
    levels[0] = leftPeak;
    levels[1] = rightPeak;
    leftPeak = 0;
    rightPeak = 0;
    return levels;
}

bool DJAudioPlayer::hasStreamFinished()
{
    return transportSource.hasStreamFinished();
}

void DJAudioPlayer::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (transportSource.hasStreamFinished())
        sendChangeMessage();
}
