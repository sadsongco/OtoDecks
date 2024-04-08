/*
 ==============================================================================
 
 PlaylistComponent.cpp
 Created: 17 Jul 2020 10:20:03am
 Author:  Nigel Powell
 
 ==============================================================================
 */

#include <JuceHeader.h>
#include <iterator>
#include <sstream>
#include <regex>
#include "PlaylistComponent.h"

//==============================================================================
PlaylistComponent::PlaylistComponent(juce::AudioFormatManager &formatManagerToUse,
                                     DJAudioPlayer &_player1,
                                     DeckGUI* &_deckGUI1,
                                     DJAudioPlayer &_player2,
                                     DeckGUI* &_deckGUI2) :
                                        formatManager(&formatManagerToUse),
                                        player1 (&_player1),
                                        player2 (&_player2)
{
    // initialise settings
    nextLibraryId = 0;
    musicLibPath = "musicLib.txt";
    loadMusicLib();
    tracksToDisplay = musicLib;
    crossfade.setName(juce::String("crossfade"));
    crossfadeTime.setName(juce::String("crossfade time"));
    autoCrossfadeToggle = true;
    autoplayToggle = false;
    
    deckGUIs.push_back(_deckGUI1);
    deckGUIs.push_back(_deckGUI2);
    
    tableComponent.getHeader().addColumn("Track Title", 1, 400);
    tableComponent.getHeader().addColumn("Length", 2, 100);
    tableComponent.getHeader().addColumn("Load To Player 1", 3, 100);
    tableComponent.getHeader().addColumn("Load To Player 2", 4, 100);
    tableComponent.getHeader().addColumn("Remove", 5, 100);
    
    tableComponent.setModel(this);
    
    addAndMakeVisible(searchInput);
    searchInput.addListener(this);
    searchInput.setTextToShowWhenEmpty("search playlist by title", warningTextColour);
    searchInput.setIndents(4, 8);
    
    addAndMakeVisible(autoPlay);
    addAndMakeVisible(autoCrossfade);
    addAndMakeVisible(crossfadeTime);
    addAndMakeVisible(loadToPlaylist);
    addAndMakeVisible(clearPlaylist);
    
    autoPlay.onClick = [this] { engageAutoplay(); };
    autoPlay.setClickingTogglesState(true);
    
    autoCrossfade.onClick = [this] { triggerAutoCrossfade(); };
    
    loadToPlaylist.onClick = [this] { openFileBrowser(); };
    
    clearPlaylist.onClick = [this] { emptyPlaylist(); };
    
    deckGUIs[0]->addChangeListener(this);
    deckGUIs[1]->addChangeListener(this);
    
    crossfade.setSliderStyle(juce::Slider::LinearBar);
    crossfade.setTextBoxIsEditable(false);
    crossfade.setNumDecimalPlacesToDisplay(0);
    crossfade.onValueChange = [this] { setCrossfade(); };
    crossfade.setRange(0.0, 1.0);
    // display position as minutes and seconds */
    crossfade.textFromValueFunction = [](double value)
    {
        int rightPC = value * 100;
        int leftPC = 100 - rightPC;
        juce::String posText = juce::String(leftPC) + "%\t" + juce::String(rightPC) + "%";
        return juce::String(posText);
    };
    crossfade.setValue(1.0);
    crossfade.setDoubleClickReturnValue(true, 0.0);
    
    crossfadeTime.setSliderStyle(juce::Slider::LinearBar);
    crossfadeTime.setRange(155, 3000);
    crossfadeTime.setValue(1000);
    crossfadeTime.setTextValueSuffix("ms");
    crossfadeTime.setNumDecimalPlacesToDisplay(0);
    crossfadeTime.setDoubleClickReturnValue(true, 1000);

    addAndMakeVisible(tableComponent);
    addAndMakeVisible(crossfade);
    addAndMakeVisible(deckGUIs[0]);
    addAndMakeVisible(deckGUIs[1]);

    autoCrossfadeInc = crossfade.getRange().getLength()/(crossfadeTime.getValue()/10) * -1;
    startTimer(10);

}

PlaylistComponent::~PlaylistComponent()
{
    stopTimer();
}

void PlaylistComponent::paint (juce::Graphics& g)
{
    g.setColour (controllerBody);
    g.drawRoundedRectangle(0.0f, 0.0f, getWidth(), getHeight(), 5, 1);  // draw an outline around the component
}

void PlaylistComponent::resized()
{
    auto guiIndent = 3;
    rowH = (getHeight() - guiIndent) / 16;
    colW = (getWidth() - guiIndent) / 5;
    searchInput.setBounds(guiIndent, guiIndent, colW, rowH - (guiIndent * 2));
    clearPlaylist.setBounds(colW * 4, guiIndent, colW, rowH - (guiIndent * 2));
    tableComponent.autoSizeColumn(3);
    tableComponent.autoSizeColumn(4);
    tableComponent.autoSizeColumn(5);
    tableComponent.setBounds(guiIndent, rowH, colW*5, rowH*6);
    loadToPlaylist.setBounds(guiIndent, rowH*7 + guiIndent, colW, rowH);
    autoPlay.setBounds(colW, rowH*7 + guiIndent, colW, rowH);
    autoCrossfade.setBounds(colW*3, rowH*7 + guiIndent, colW, rowH);
    crossfadeTime.setBounds(colW*4, rowH*7 + guiIndent, colW, rowH);
    crossfade.setBounds(colW*2, rowH*7 + guiIndent, colW, rowH);
    deckGUIs[0]->setBounds(guiIndent, (getHeight()/2) + guiIndent, getWidth()/2 - (guiIndent * 2), getHeight()/2 - (guiIndent * 2));
    deckGUIs[1]->setBounds(getWidth()/2 + guiIndent, (getHeight()/2) + guiIndent, getWidth()/2 - (guiIndent * 2), getHeight()/2 - (guiIndent * 2));
}

void PlaylistComponent::setColourPalette(juce::Colour &_controllerBackground,
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
}

int PlaylistComponent::getNumRows()
{
    return tracksToDisplay.size();
}
void PlaylistComponent::paintRowBackground(Graphics & g,
                                           int rowNumber,
                                           int width,
                                           int height,
                                           bool rowIsSelected)
{
    const float alpha = (rowNumber % 2 == 0 ? 1.0f : 0.5f);
    if (rowIsSelected)
    {
        g.fillAll(controllerIndicator);
    }
    else
    {
        g.fillAll(controllerBackground.withAlpha(alpha));
    }
}
void PlaylistComponent::paintCell (Graphics & g,
                                   int rowNumber,
                                   int columnId,
                                   int width,
                                   int height,
                                   bool rowIsSelected)
{
    g.setColour(infoTextColour);
    float controlIndent = 2.0f;
    if (columnId == 1)
        g.drawText (tracksToDisplay[rowNumber].title, 2, 0, width - 4, height, Justification::centredLeft, true);
    if (columnId == 2)
        g.drawText (lengthToMinutesAndSeconds(tracksToDisplay[rowNumber].length), 2, 0, width - 4, height, Justification::centred, false);
    if (columnId == 3 || columnId == 4)
    {
        g.setColour(controllerBody);
        g.drawRoundedRectangle(controlIndent, controlIndent, width - (controlIndent * 2), height - (controlIndent * 2), 4, 2);
        rowIsSelected ? g.setColour(juce::Colours::black) : g.setColour(controllerIndicator);
        g.setFont(g.getCurrentFont().getHeight() * 0.8f);
        g.drawText("load to player " + std::to_string(columnId - 2), controlIndent, controlIndent, width - (controlIndent * 2), height - (controlIndent * 2), juce::Justification::centred);
    }
    if (columnId == 5)
    {
        g.setColour(controllerBody);
        g.drawRoundedRectangle(controlIndent, controlIndent, width - (controlIndent * 2), height - (controlIndent * 2), 4, 2);
        rowIsSelected ? g.setColour(juce::Colours::black) : g.setColour(controllerIndicator);
        g.setFont(g.getCurrentFont().getHeight() * 0.8f);
        g.drawText("remove", controlIndent, controlIndent, width - (controlIndent * 2), height - (controlIndent * 2), juce::Justification::centred);
    }
}

Component* PlaylistComponent::refreshComponentForCell (int rowNumber,
                                                       int columnId,
                                                       bool isRowSelected,
                                                       Component *existingComponentToUpdate)
{
    return existingComponentToUpdate;
}

/* ========================================= */
/* ====== listeners and event lambdas ====== */
/* ========================================= */

void PlaylistComponent::cellClicked(int rowNumber, int columnId, const MouseEvent&)
{
    if (columnId == 3 || columnId == 4)
    {
        deckGUIs[columnId - 3]->currentTrackName = tracksToDisplay[rowNumber].title;
        deckGUIs[columnId - 3]->loadFile(tracksToDisplay[rowNumber].trackURL);
    }
    if (columnId == 5)
    {
        long int libraryIdToRemove = tracksToDisplay[rowNumber].libraryId;
        removeFromLibrary(libraryIdToRemove);
    }
}

void PlaylistComponent::cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent&)
{
    loadIfNotPlaying(rowNumber);
}

void PlaylistComponent::returnKeyPressed(int lastRowSelected)
{
    loadIfNotPlaying(lastRowSelected);
}
void PlaylistComponent::deleteKeyPressed(int lastRowSelected)
{
    long int libraryIdToRemove = tracksToDisplay[lastRowSelected].libraryId;
    removeFromLibrary(libraryIdToRemove);
}

void PlaylistComponent::setCrossfade()
{
    double player2Ratio = crossfade.getValue();
    double player1Ratio = 1 - player2Ratio;
    deckGUIs[0]->setCrossfadeRatio(player1Ratio);
    deckGUIs[1]->setCrossfadeRatio(player2Ratio);
}

void PlaylistComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (autoPlay.getToggleStateValue() == 1)
    {
        for (int i = 0; i < deckGUIs.size(); ++i)
        {
            DeckGUI* dG = deckGUIs[i];
            DeckGUI* otherDG = deckGUIs[!i];
            if (source == dG && dG->streamNearlyEnded && !dG->streamEnded)
            {
                if (!otherDG->isPlaying())
                    otherDG->play();
                autoCrossfadeInc = crossfade.getRange().getLength()/(crossfadeTime.getValue()/10) * (1 - (2 * i));
                autoCrossfadeToggle = true;
            }
            else if (source == dG && dG->streamEnded)
            {
                Track temp = musicLib[0];
                dG->loadFile(temp.trackURL);
                dG->currentTrackName = temp.title;
                musicLib.erase(musicLib.begin());
                musicLib.push_back(temp);
                saveMusicLib();
                filterTracksToDisplayByTitle(searchInput.getText().toStdString());
                dG->streamEnded = false;
                dG->streamNearlyEnded = false;
                dG->toTrackStart();
            }
        }
    }
    for (int i = 0; i < deckGUIs.size(); i ++)
    {
        if (deckGUIs[!i]->targetBpm != deckGUIs[i]->bpm)
            deckGUIs[!i]->targetBpm = deckGUIs[i]->bpm;
    }
}

void PlaylistComponent::timerCallback()
{
    if (autoCrossfadeToggle && crossfade.getValue() <= crossfade.getRange().getEnd() && crossfade.getValue() >= crossfade.getRange().getStart())
    {
        crossfade.setValue(crossfade.getValue() + autoCrossfadeInc);
        if (crossfade.getValue() >= crossfade.getRange().getEnd())
            crossfade.setValue(crossfade.getRange().getEnd());
        else if (crossfade.getValue() <= crossfade.getRange().getStart())
            crossfade.setValue(crossfade.getRange().getStart());
    }
    if (autoCrossfadeToggle && (crossfade.getValue() == crossfade.getRange().getEnd() || crossfade.getValue() == crossfade.getRange().getStart()))
    {
        autoCrossfadeToggle = false;
    }
}

void PlaylistComponent::textEditorTextChanged(TextEditor& textEditor)
{
    if (textEditor.getText() == "")
    {
        tracksToDisplay = musicLib;
        tableComponent.updateContent();
        repaint();
    }
    else
        filterTracksToDisplayByTitle(textEditor.getText().toStdString());
}

/* ================================================ */
/* ====== load file and manage music library ====== */
/* ================================================ */

bool PlaylistComponent::isInterestedInFileDrag (const juce::StringArray &files)
{
    return true;
}
void PlaylistComponent::filesDropped (const StringArray &files, int x, int y)
{
    if (files.size() >= 1)
    {
        for (auto file : files)
        {
            loadFileToMusicLib(file);
        }
    }
}

void PlaylistComponent::backgroundClicked(const MouseEvent &mEv)
{
    FileChooser chooser("Add audio file to playlist...");
    if (chooser.browseForFileToOpen())
    {
        loadFileToMusicLib(chooser.getResult());
    }
}

void PlaylistComponent::openFileBrowser()
{
    juce::FileChooser chooser("Select audio file...");
    if (chooser.browseForFileToOpen())
    {
        loadFileToMusicLib(chooser.getResult());
    }
}

void PlaylistComponent::loadMusicLib()
{
    std::ifstream musicLibFile;
    musicLibFile.open(musicLibPath);
    if (musicLibFile.is_open())
    {
        for ( std::string line; getline(musicLibFile, line); )
        {
            Track track = tokeniseMusicLibLine(line);
            if (track.libraryId > nextLibraryId)
                nextLibraryId = track.libraryId + 1;
            musicLib.push_back(track);
        }
    }
    musicLibFile.close();
}

void PlaylistComponent::saveMusicLib()
{
    // erases and re-writes the whole file each time to account for order changes etc.
    std::ofstream musicLibFile;
    musicLibFile.open(musicLibPath, std::ofstream::trunc);
    if (musicLibFile.is_open())
    {
        std::string writeString = "";
        for (Track track : musicLib)
        {
            std::string writeLine = std::to_string(track.libraryId) + "\t" + track.title.toStdString() + "\t" + std::to_string(track.length) + "\t" + track.trackURL.toString(false).toStdString() + "\n";
            writeString += writeLine;
        }
        musicLibFile << writeString;
    }
    musicLibFile.close();
}

PlaylistComponent::Track PlaylistComponent::tokeniseMusicLibLine(std::string line)
{
    Track trackFromLine;
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (std::getline(iss, token, '\t'))
    {
        tokens.push_back(token);
    }
    trackFromLine.libraryId = std::stol(tokens[0]);
    trackFromLine.title = juce::String(tokens[1]);
    trackFromLine.length = std::stof(tokens[2]);
    trackFromLine.trackURL = juce::URL(tokens[3]);
    return trackFromLine;
}

void PlaylistComponent::loadFileToMusicLib(juce::File file)
{
    auto* testReader = formatManager->createReaderFor(file);
    if (testReader != nullptr)
    {
        juce::File fileToAdd = File{file};
        Track track;
        track.libraryId = nextLibraryId;
        track.title = fileToAdd.getFileName();
        track.trackURL = URL{File{file}};
        track.length = testReader->lengthInSamples / testReader->sampleRate;
        musicLib.push_back(track);
        tableComponent.updateContent();
        repaint();
        saveMusicLib();
        ++nextLibraryId;
        filterTracksToDisplayByTitle(searchInput.getText().toStdString());
    }
    delete testReader;
}

/* ========================================== */
/* ====== state reporters and updaters ====== */
/* ========================================== */

bool PlaylistComponent::isPlaying()
{
    if (deckGUIs[0]->isPlaying() || deckGUIs[1]->isPlaying())
        return true;
    else
        return false;
}

void PlaylistComponent::engageAutoplay()
{
    if (musicLib.size() > 0)
    {
        autoplayToggle = !autoplayToggle;
        if (autoplayToggle)
        {
            for (int i = 0; i < deckGUIs.size(); ++i)
            {
                DeckGUI* dG = deckGUIs[i];
                if (!dG->fileLoaded)
                {
                    Track temp = musicLib[0];
                    dG->loadFile(temp.trackURL);
                    dG->currentTrackName = temp.title;
                    musicLib.erase(musicLib.begin());
                    musicLib.push_back(temp);
                    saveMusicLib();
                    filterTracksToDisplayByTitle(searchInput.getText().toStdString());
                }
            }
            if (!deckGUIs[0]->isPlaying() && !deckGUIs[1]->isPlaying())
                deckGUIs[0]->play();
        }
    }
    else
        autoPlay.setToggleState(false, juce::NotificationType::dontSendNotification);
}

void PlaylistComponent::triggerAutoCrossfade()
{
    autoCrossfadeInc = crossfade.getRange().getLength()/(crossfadeTime.getValue()/10) * (1 - (2 * std::round(crossfade.getValue())));
    autoCrossfadeToggle = true;
    for (DeckGUI* dg : deckGUIs)
        if (!dg->isPlaying()) {dg->play();}
}

/* ======================= */
/* ====== utilities ====== */
/* ======================= */

juce::String PlaylistComponent::lengthToMinutesAndSeconds(float length)
{
    juce::String secStr, minStr;
    int seconds = ((int)length)%60;
    if (seconds < 10)
        secStr = "0" + juce::String(std::to_string(seconds)) + "\"";
    else
        secStr = juce::String(std::to_string(seconds)) + "\"";
    int minutes = ((int)length - seconds)/60;
    if (minutes < 1)
        minStr = "";
    else
        minStr = juce::String(std::to_string(minutes)) + "'";
    juce::String minAndSec =  minStr + secStr;
    return minAndSec;
}

void PlaylistComponent::filterTracksToDisplayByTitle(std::string searchTerm)
{
    if (searchTerm == "")
    {
        tracksToDisplay.clear();
        tracksToDisplay = musicLib;
    }
    else
    {
        tracksToDisplay.clear();
        std::regex regexSearchTerm(searchTerm, std::regex_constants::icase);
        // perform linear search on tracks vector testing for match
        for (Track track : musicLib)
        {
            if (std::regex_search(track.title.toStdString(), regexSearchTerm))
                tracksToDisplay.push_back(track);
        }
    }
    tableComponent.updateContent();
    repaint();
}

void PlaylistComponent::loadIfNotPlaying(int index)
{
    // if track double clicked, load to a player if it's not playing
    int playerTarget = -1;
    if (!deckGUIs[0]->isPlaying())
        playerTarget = 0;
    else if (!deckGUIs[1]->isPlaying())
        playerTarget = 1;
    if (playerTarget > -1)
    {
        deckGUIs[playerTarget]->currentTrackName = tracksToDisplay[index].title;
        deckGUIs[playerTarget]->loadFile(tracksToDisplay[index].trackURL);
    }
}

void PlaylistComponent::removeFromLibrary(long int libraryIdToRemove)
{
    // linear search through musicLib, remove track with this unique id
    for (int i = 0; i < musicLib.size(); ++i)
    {
        if (musicLib[i].libraryId == libraryIdToRemove)
        {
            musicLib.erase(musicLib.begin() + i);
            break;
        }
    }
    saveMusicLib();
    filterTracksToDisplayByTitle(searchInput.getText().toStdString());
}

void PlaylistComponent::emptyPlaylist()
{
    musicLib.clear();
    saveMusicLib();
    filterTracksToDisplayByTitle("");
}
