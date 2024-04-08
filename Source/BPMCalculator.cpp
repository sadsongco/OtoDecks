/*
  ==============================================================================

    bpmCalculator.cpp
    Created: 18 Aug 2020 2:09:01pm
    Author:  Nigel Powell

  ==============================================================================
*/

#include "BPMCalculator.h"
#include <iostream>

BPMCalculator::BPMCalculator()
{
    samplesPerBlock = 1024;
    blocksPerSecond = 0;
    localBeatCounter = 0;
    localPeakCounter = 0;
    blockCounter = 0;
    localBlockCounter = 0;
    _bpm = -1.0f;
    startTimer(5000);
}

BPMCalculator::~BPMCalculator()
{
    
}

void BPMCalculator::getBlockEnergy()
{
    windowPeriod = sampleRate * 5 / samplesPerBlock;
    blocksPerSecond = (int)(windowPeriod / 5);
    // iterate through current block, calculate energy, push into energy array
    float blockEnergy = 0;
    while(!blockBuffer.empty())
    {
        blockEnergy += blockBuffer.front().first * blockBuffer.front().first + blockBuffer.front().second * blockBuffer.front().second;
        blockBuffer.pop();
    }
    energyBuffer.push(blockEnergy);
    ++blockCounter;
    ++localBlockCounter;
    // compare this block energy with the previous second if there is enough in the energyBuffer
    if (energyBuffer.size() >= blocksPerSecond)
    {
        averageLocalEnergy();
        energyVariance();
        energyBuffer.pop();
        calculateConstant();
        if (blockEnergy > beatDetectConstant * average)
        {
            ++localPeakCounter;
            if (localPeakCounter == 2)
            {
                localPeakCounter = 0;
                ++localBeatCounter;
                ++beatCounter;
            }
        }
        else
        {
            localPeakCounter = 0;
        }
        if (sampleRate > 0 && localBlockCounter > sampleRate * 5 / samplesPerBlock)
        {
            calculateBPM();
            localBlockCounter = 0;
            localBeatCounter = 0;
        }
    }
}
void BPMCalculator::averageLocalEnergy()
{
    // calculate average energy in window
    average = 0;
    for (int i = 0; i < blocksPerSecond; ++i)
    {
        float temp = energyBuffer.front();
        average += temp;
        energyBuffer.pop();
        energyBuffer.push(temp);
    }
    if (blocksPerSecond > 0)
        average /= blocksPerSecond;
    else
        average = 0;
}

void BPMCalculator::energyVariance()
{
    // calculate energy variance in window
    variance = 0;
    for (int i = 0; i < blocksPerSecond; ++i)
    {
        float temp = energyBuffer.front();
        variance += (average - temp) * (average - temp);
        energyBuffer.pop();
        energyBuffer.push(temp);
    }
    if (blocksPerSecond > 0)
        variance /= blocksPerSecond;
    else
        variance = 0;
}

void BPMCalculator::calculateConstant()
{
    // calculate constant
    beatDetectConstant = (cMult * variance) + cAdd;
}

void BPMCalculator::calculateBPM()
{
    float beatsPerMinute = 0;
    float timeTakenForBeatsDetected = localBlockCounter * samplesPerBlock / sampleRate;
    beatsPerMinute = localBeatCounter * 60 / timeTakenForBeatsDetected;
    instantBpm.push_back(beatsPerMinute);
}

void BPMCalculator::timerCallback()
{
    double bpmTotal = 0;
    float bpmSize = instantBpm.size();
    if (bpmSize > 0)
    {
        for (auto bpm : instantBpm)
        {
            bpmTotal += bpm;
        }
        _bpm = bpmTotal / bpmSize;
    }
    if (_bpm < 1000 && _bpm > 1)
    {
        while (_bpm > 180)
            _bpm /= 2;
        while (_bpm < 90)
            _bpm *= 2;
    }
}
