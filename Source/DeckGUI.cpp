/*
  ==============================================================================

    DeckGUI.cpp
    Created: 27 Jun 2020 11:00:50am
    Author:  Nigel Powell

  ==============================================================================
*/

#include "DeckGUI.h"
#include <vector>

//==============================================================================
DeckGUI::DeckGUI(DJAudioPlayer* _player,
                 AudioFormatManager &formatManagerToUse,
                 AudioThumbnailCache &cacheToUse,
                 short int _guiID
                 ) :    guiID(_guiID),
                        player(_player),
                        waveformDisplay(formatManagerToUse, cacheToUse)
{
    // initialise settings
    bpm = -1;
    fileLoaded = false;
    getLookAndFeel().setDefaultSansSerifTypefaceName("Verdana");
    playerStatus = "No file loaded";
    posSlider.setName(juce::String("posSlider"));
    posSlider.setLookAndFeel(&waveformDisplay);
    
    // set up GUI elements */
    addAndMakeVisible(playButton);
    addAndMakeVisible(returnButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(matchTempoButton);
    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(posSlider);
    
    // add button listeners */
    playButton.onClick = [this] { play(); };
    returnButton.onClick = [this] { toTrackStart(); };
    loadButton.onClick = [this] { openFileChooser(); };
    matchTempoButton.onClick = [this] { matchTempo(); };
    player->addChangeListener(this);
    
    // volume slider and label */
    volSlider.setTextBoxIsEditable(false);
    volSlider.setTextValueSuffix("%");
    volSlider.addListener(this);
    volSlider.setRange(0, 120);
    volSlider.setValue(100);
    volSlider.setDoubleClickReturnValue(true, 100);
    volSlider.setNumDecimalPlacesToDisplay(0);
//    volSlider.setLookAndFeel(&sliderLookAndFeel);
    
    volLabel.setText("Volume", juce::dontSendNotification);
    volLabel.attachToComponent(&volSlider, true);

    // speed slider and label */
    speedSlider.setTextBoxIsEditable(false);
    speedSlider.addListener(this);
    speedSlider.setRange(0.5, 2);
    // display speed as a percentage */
    speedSlider.textFromValueFunction = [](double value)
    {
        return juce::String((int)(value * 100)) + "%";
    };
    speedSlider.setValue(1);
    speedSlider.setDoubleClickReturnValue(true, 1);
    
    speedLabel.setText("Speed", juce::dontSendNotification);
    speedLabel.attachToComponent(&speedSlider, true);
    
    // position slider and label */
    posSlider.setSliderStyle(juce::Slider::LinearBar);
    posSlider.setTextBoxIsEditable(false);
    posSlider.setNumDecimalPlacesToDisplay(0);
    posSlider.addListener(this);
    posSlider.setRange(0.0, 1.0);
    posSlider.setValue(0.0);
    posSlider.setDoubleClickReturnValue(true, 0.0);
    // label slider */
    posLabel.setText("Position", juce::dontSendNotification);
    posLabel.attachToComponent(&posSlider, true);
    
    // level meters for this player */
    addAndMakeVisible(levelL);
    addAndMakeVisible(levelR);
    
    startTimer(100);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
    posSlider.setLookAndFeel(nullptr);
}

void DeckGUI::paint (Graphics& g)
{
    // set up variables for position to allow for easy rearrangement of GUI
    int c, r, w, h;                                                      // column, row, width, height - count from 0
    g.setColour (controllerBody);
    g.drawRoundedRectangle(0, 0, getWidth(), getHeight(), 5.0f, 2.0f);   // draw an outline around the component
    g.setColour(infoTextColour);
    // show track time (5, 0) to (7, 0)
    c = 5; r = 0; w = 2; h = 1;
    g.drawText(secondsToMinutesAndSeconds(posSlider.getValue()), colW * c + padding, rowH * r + padding, colW * w, rowH * h, Justification::centredLeft, true);
    // show track name (1, 1) to (1, 6)
    c = 1; r = 1; w = 7; h = 1;
    if (currentTrackName != "")
    {
        g.setFont(18.0f);
        g.drawText(currentTrackName, colW * c + padding, rowH * r + padding, colW * w, rowH * h, Justification::centredLeft, true);
    }
    g.setColour(warningTextColour);
    // status (1, 2)
    c = 1; r = 2; w = 2; h = 1;
    g.setFont(12.0f);
    g.drawText(playerStatus, colW * c + padding, rowH * r + padding, colW * w, rowH * h, Justification::centredLeft, true);
    // bpm (4, 2)
    c = 3; r = 2; w = 2; h = 1;
    if (bpm > 0)
        g.drawText("est.BPM: " + String(bpm), colW * c + padding, rowH * r + padding, colW * w, rowH * h, Justification::centredLeft, true);
    // paint custom button backgrounds
    g.setColour(controllerBackground);
    g.fillRoundedRectangle(playButton.getX(), playButton.getY(), playButton.getWidth(), playButton.getHeight(), 5.0f);
    g.fillRoundedRectangle(returnButton.getX(), returnButton.getY(), returnButton.getWidth(), returnButton.getHeight(), 5.0f);
    g.fillRoundedRectangle(loadButton.getX(), loadButton.getY(), loadButton.getWidth(), loadButton.getHeight(), 5.0f);
    if (bpm > 0 && targetBpm > 0)
    {
        c = 5; r = 2; w = 2; h = 1;
        g.setColour(warningTextColour);
        g.drawText("match BPM", colW * c + padding - 2, rowH * r + padding, colW * w, rowH * h, Justification::centredRight, true);
        g.setColour(controllerBackground);
        g.fillRoundedRectangle(matchTempoButton.getX(), matchTempoButton.getY(), matchTempoButton.getWidth(), matchTempoButton.getHeight(), 5.0f);
    }
    // paint custom button outlines
    g.setColour(controllerBody);
    g.drawRoundedRectangle(playButton.getX(), playButton.getY(), playButton.getWidth(), playButton.getHeight(), 5.0f, 2.0f);
    g.drawRoundedRectangle(returnButton.getX(), returnButton.getY(), returnButton.getWidth(), returnButton.getHeight(), 5.0f, 2.0f);
    g.drawRoundedRectangle(loadButton.getX(), loadButton.getY(), loadButton.getWidth(), loadButton.getHeight(), 5.0f, 2.0f);
    if (bpm > 0 && targetBpm > 0)
        g.drawRoundedRectangle(matchTempoButton.getX(), matchTempoButton.getY(), matchTempoButton.getWidth(), matchTempoButton.getHeight(), 5.0f, 2.0f);
    // paint custom button icons
    g.setColour(controllerIndicator);
    if (!isPlaying())
        g.fillPath(playIcon);
    else
        g.fillPath(pauseIcon);
    g.fillPath(returnIcon);
    g.fillPath(loadIcon);
    if (bpm > 0 && targetBpm > 0)
        g.strokePath(matchTempoIcon, PathStrokeType (2.0f));
}

void DeckGUI::resized()
{
    // set positions for all GUI elements and icons
    padding = getWidth() / 50;
    double componentX = 0;
    // 8 x 8 grid for layout
    rowH = (getHeight() - (padding * 2)) / 8;
    colW = (getWidth() - (padding * 2)) / 8;
    int c, r, w, h;                                                     // column, row, width, height - count from 0
    // fixed position elements
    // posSlider (1, 5) to (8, 6)
    c = 1; r = 5; w = 7; h = 3;
    posSlider.setBounds(colW * c + padding, rowH * r  + padding, colW * w, rowH * h);
    // volume dial (1, 3) to (4, 5)
    c = 1; r = 3; w = 3; h = 2;
    volSlider.setBounds(colW * c + padding, rowH * r  + padding, colW * w, rowH * h);
    // speed dial (5, 3) to (8, 5)
    c = 5; r = 3; w = 3; h = 2;
    speedSlider.setBounds(colW * c + padding, rowH * r  + padding, colW * w, rowH * h);
    // meter layout variables
    meterWidth = colW * 0.25;
    meterMargin = colW * 0.2;
    meterBetween = colW * 0.05;
    // L & R meters (0, 0) to (0, 3)
    c = 0; r = 0; w = 1; h = 3;
    levelL.setBounds(componentX + colW * c + meterMargin, rowH * r + padding, meterWidth, rowH * h);
    levelR.setBounds(componentX + colW * c + meterMargin + meterWidth + meterBetween, rowH * r + padding, meterWidth, rowH * h);
    // playButton (1, 0)
    c = 1; r = 0; w = 1; h = 1;
    playButton.setBounds(componentX + colW * c + padding, rowH * r  + padding, colW * w, rowH * h);
    // returnButton (2, 0)
    c = 2; r = 0; w = 1; h = 1;
    returnButton.setBounds(componentX + colW * c + padding, rowH * r  + padding, colW * w, rowH * h);
    // loadButton (3, 0)
    c = 3; r = 0; w = 1; h = 1;
    loadButton.setBounds(componentX + colW * c + padding, rowH * r  + padding, colW * w, rowH * h);
    // matchTempoButton (7, 2)
    c = 7; r = 2; w = 1; h = 1;
    matchTempoButton.setBounds(componentX + colW * c + padding, rowH * r  + padding, colW * w, rowH * h);
    // common button variables
    float iconGutterW = colW / 5, iconGutterH = rowH / 5;
    // set up playButton icons
    playIcon.clear();
    pauseIcon.clear();
    playIcon.addTriangle(playButton.getX() + iconGutterW, playButton.getY() + iconGutterH,
                         playButton.getX() + playButton.getWidth() - iconGutterW, playButton.getY() + (playButton.getHeight() / 2),
                         playButton.getX() + iconGutterW, playButton.getY() + playButton.getHeight() - iconGutterH);
    pauseIcon.addRectangle(playButton.getX() + (playButton.getWidth() / 5), playButton.getY() + iconGutterH,
                           playButton.getWidth() / 5, playButton.getHeight() - (iconGutterH * 2));
    pauseIcon.addRectangle(playButton.getX() + (playButton.getWidth() * 3 / 5), playButton.getY() + iconGutterH,
                           playButton.getWidth() / 5, playButton.getHeight() - (iconGutterH * 2));
    // set up return button icon
    returnIcon.clear();
    returnIcon.addRectangle(returnButton.getX() + (returnButton.getWidth() / 5), returnButton.getY() + iconGutterH,
                            returnButton.getWidth() / 5, returnButton.getHeight() - (iconGutterH * 2));
    returnIcon.addTriangle(returnButton.getX() + (returnButton.getWidth() * 2 / 5), returnButton.getY() + returnButton.getHeight() / 2,
                           returnButton.getX() + returnButton.getWidth() - iconGutterW, returnButton.getY() + iconGutterH,
                           returnButton.getX() + returnButton.getWidth() - iconGutterW, returnButton.getY() + returnButton.getHeight() - iconGutterH);
    // set up load button icon
    loadIcon.clear();
    juce::Point<float> temp1{(float)(loadButton.getX() + (loadButton.getWidth() / 2)), float(loadButton.getY() + iconGutterH)};
    juce::Point<float> temp2{float(loadButton.getX() + (loadButton.getWidth() / 2)), float(loadButton.getY() + loadButton.getHeight() - (int)iconGutterH)};
    juce::Line<float> tempLine{temp1, temp2};
    loadIcon.addArrow(tempLine, iconGutterW, iconGutterW, iconGutterH);
    // set up match tempo button icon
    matchTempoIcon.clear();
    double gridW = matchTempoButton.getWidth() / 12;
    double gridH = matchTempoButton.getHeight() / 6;
    double x = matchTempoButton.getX();
    double y = matchTempoButton.getY();
    if (guiID % 2 == 1)
    {
        gridW *= -1;
        x += matchTempoButton.getWidth();
    }
    matchTempoIcon.startNewSubPath(x + gridW * 3, y + gridH * 5);
    matchTempoIcon.lineTo(x + gridW * 7, y + gridH * 5);
    matchTempoIcon.lineTo(x + gridW * 6, y + gridH * 1);
    matchTempoIcon.lineTo(x + gridW * 4, y + gridH * 1);
    matchTempoIcon.lineTo(x + gridW * 3, y + gridH * 5);
    matchTempoIcon.closeSubPath();
    matchTempoIcon.startNewSubPath(x + gridW * 5, y + gridH * 4);
    matchTempoIcon.lineTo(x + gridW * 7, y + gridH * 2);
    matchTempoIcon.closeSubPath();
    matchTempoIcon.startNewSubPath(x + gridW * 8, y + gridH * 2);
    matchTempoIcon.lineTo(x + gridW * 11, y + gridH * 3);
    matchTempoIcon.lineTo(x + gridW * 8, y + gridH * 4);
    colW = (getWidth() - (padding * 2)) / 8;
}

void DeckGUI::setColourPalette(juce::Colour &_controllerBackground,
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
    waveformDisplay.setColourPalette(controllerBackground,
                                     controllerBody,
                                     controllerIndicator,
                                     infoTextColour,
                                     warningTextColour);
}

void DeckGUI::buttonClicked(Button* button)
{
    
}

void DeckGUI::sliderValueChanged(Slider* slider)
{
        if (slider == &volSlider)
            player->setGain(slider->getValue()/100);
        if (slider == &speedSlider)
            player->setSpeed(slider->getValue());
        if (slider == &posSlider)
            player->setPosition(slider->getValue());
}

bool DeckGUI::isInterestedInFileDrag (const StringArray &files)
{
    std::cout << "DeckGUI::isInterestedInFileDrag" << std::endl;
    return true;
}

void DeckGUI::filesDropped (const StringArray &files, int x, int y)
{
    if (files.size() == 1)
    {
        loadFile(URL{File{files[0]}});
        fileLoaded = true;
        currentTrackName = File{files[0]}.getFileName();
    }
}

void DeckGUI::timerCallback()
{
    if (player->isPlaying())
    {
        // update slider position as track plays
        posSlider.setValue(player->getPosition(), juce::dontSendNotification);
        // update level meters
        std::vector<double> audioLevels = player->getLevels();
        levelL.displayLevel(audioLevels[0]);
        levelR.displayLevel(audioLevels[1]);
        // send message if less than 5 seconds from the end
        if (player->getLengthInSeconds() - player->getPosition() < 5 && !streamNearlyEnded)
        {
            streamNearlyEnded = true;
            sendChangeMessage();
        }
    }
    else
    {
        levelL.displayLevel(0);
        levelR.displayLevel(0);
    }
    if ((bpm != (int)player->currentBPM && isPlaying()) || targetBpm == -1)
    {
        bpm = (int)player->currentBPM;
        sendChangeMessage();
    }
    repaint();
}

bool DeckGUI::keyPressed (const KeyPress &key)
{
    if (key.getKeyCode() == 32)
    {
        play();
    }
    return true;
}

void DeckGUI::loadFile(juce::URL url)
{
    player->loadURL(url);
    playerStatus = "Queued";
    waveformDisplay.loadURL(url);
    // update slider with length in seconds of new file
    posSlider.setRange(0, player->getLengthInSeconds());
    posSlider.setNumDecimalPlacesToDisplay(1);
    // initialise gain and crossfade
    player->setGain(volSlider.getValue()/100);
    player->setCrossfadeRatio(currentCrossfadeRatio);
    // initialise bpm, stream ended and stream nearly ended
    bpm = -1;
    sendChangeMessage();
    streamEnded = false;
    streamNearlyEnded = false;
    fileLoaded = true;
}

// reduce volume when dragging through track
void DeckGUI::sliderDragStarted(Slider* slider)
{
    if (slider == &posSlider)
        player->dimWhileScrubbing = true;
}

void DeckGUI::sliderDragEnded(Slider* slider)
{
    if (slider == &posSlider)
        player->dimWhileScrubbing = false;
}

void DeckGUI::setCrossfadeRatio(double ratio)
{
    currentCrossfadeRatio = ratio;
    player->setCrossfadeRatio(currentCrossfadeRatio);
}

bool DeckGUI::isPlaying()
{
    return player->isPlaying();
}

void DeckGUI::play()
{
    if (streamEnded)
    {
        posSlider.setValue(0);
        streamEnded = false;
        streamNearlyEnded = false;
        playerStatus = "Queued";
    }
    if (player->isPlaying())
    {
        player->stop();
        if (player->getPosition() > 0)
            playerStatus = "Stopped";
        else
            playerStatus = "Queued";
    }
    else if (fileLoaded)
    {
        player->start();
        playerStatus = "Playing";
    }
}
void DeckGUI::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == player && player->getPosition() >= player->getLengthInSeconds() && !streamEnded)
    {
        streamEnded = true;
        sendChangeMessage();
    }
}

void DeckGUI::toTrackStart()
{
    posSlider.setValue(0);
}

void DeckGUI::openFileChooser()
{
    FileChooser chooser("Select audio file...");
    if (chooser.browseForFileToOpen())
    {
        loadFile(URL{chooser.getResult()});
        currentTrackName = chooser.getResult().getFileName();
    }
}

void DeckGUI::matchTempo()
{
    if (bpm > 0 && targetBpm > 0)
    {
        double tempoRatio = (double)targetBpm / (double)bpm;
        std::cout << "DeckGUI::matchTempo: tempoRatio: " << tempoRatio << std::endl;
        if (tempoRatio < speedSlider.getMinimum()) tempoRatio = speedSlider.getMinimum();
        if (tempoRatio > speedSlider.getMaximum()) tempoRatio = speedSlider.getMaximum();
        speedSlider.setValue(tempoRatio);
    }
}

juce::String DeckGUI::secondsToMinutesAndSeconds(double value)
{
    juce::String posText = "";
    int seconds = (int)value % 60;
    int minutes = ((int)value - seconds)/60;
    if (minutes > 0)
        posText += juce::String(minutes) + "'";
    if (seconds < 10 && minutes > 1)
        posText += "0" + juce::String(seconds);
    else
        posText += juce::String(seconds);
    double fracSec = (value - (minutes*60) - seconds) * 10;
    posText += "." + juce::String((int)fracSec) + "\"";
    return juce::String(posText);
}
