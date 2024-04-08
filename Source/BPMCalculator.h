/*
  ==============================================================================

    bpmCalculator.h
    Created: 18 Aug 2020 2:09:01pm
    Author:  Nigel Powell
    Simple algorithm at https://mziccard.me/2015/05/28/beats-detection-algorithms-1/

  ==============================================================================
*/
#pragma once

#include <queue>
#include <utility>
#include <JuceHeader.h>

class BPMCalculator : public Timer
{
public:
    BPMCalculator();
    ~BPMCalculator();
    float sampleRate;                                   // sample rate of the current file
    std::queue<std::pair<float, float>> blockBuffer;    // a block of  L & R samples
    int samplesPerBlock;                                // maximum size of a block
    float _bpm;                                         // calculated bpm
    int localPeakCounter;                               // the number of level peaks detected per window
    int localBeatCounter;                               // the number of beats detected per window
    int beatCounter;                                    // number of beats detected
    std::vector<float> instantBpm;                      // a vector of all the instantaneous bpms
    /**
     BPMCalculator::getBlockEnergy()
     Input                  none
     Output                 none
     calculates the energy in the current block
     stores to BPMCalculator::windowEnergy
     clears block
     */
    void getBlockEnergy();
private:
    // pure virtual Timer
    void timerCallback() override;
    /**
     BPMCalculator::averageLocalEnergy()
     Input                  none
     Output                 none
     Calculates the average energy in the current window
     Pops off the oldest stored block energy
     */
    void averageLocalEnergy();
    
    /**
     BPMCalculator::energyVariance()
     Input                  none
     Output                 none
     Iterates through the blocks and calculates
     the energy variance in the current window
     */
    void energyVariance();
    
    /**
     BPMCalculator::calculateConstant()
     Input                  none
     Output                 none
     Calculates the beat detection constant from the variance and
     multiplication and addition constants, stores in BPMCalculator::beatDetectConstant
     */
    void calculateConstant();
    
    /**
     BPMCalulculator::caclulateBPM()
     Input                  none
     Output                 none
     Calculates the beats per minute from the counted beats
     Saves into BPMCalculator::beatsPerMinute, and pushes to the vector of bpms
     */
    void calculateBPM();

    int blocksPerSecond;                                // maximum size of a window
    std::queue<float> energyBuffer;                     // a window of energies calculated from blocks
    float windowPeriod;                                 // the precise amount of time in a window (~1s)
    int blockCounter;                                   // number of blocks read so far
    int localBlockCounter;                              // number of blocks read before bpm calculation
    float average;                                      // the average energy of the current window
    float variance;                                     // average energy variance of current window
    float beatDetectConstant;                           // calculated constant for beat detection algorithm
    // constants taken from algorithm linked to in the header
    static constexpr float cMult = -0.0000075;
    static constexpr float cAdd = 1.5142857;
};
