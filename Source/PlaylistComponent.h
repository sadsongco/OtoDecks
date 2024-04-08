/*
 ==============================================================================
 
 PlaylistComponent.h
 Created: 17 Jul 2020 10:20:03am
 Author:  Nigel Powell
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include <fstream>
#include "DJAudioPlayer.h"
#include "DeckGUI.h"

//==============================================================================
/*
 */
class PlaylistComponent  :  public juce::Component,
                            public juce::TableListBoxModel,
                            public juce::FileDragAndDropTarget,
                            public juce::TextEditor::Listener,
                            public juce::ChangeListener,
                            public juce::Timer
{
public:
    PlaylistComponent(juce::AudioFormatManager &formatManagerToUse,
                      DJAudioPlayer &_player1,
                      DeckGUI* &_deckGUI1,
                      DJAudioPlayer &_player2,
                      DeckGUI* &_deckGUI2);
    ~PlaylistComponent() override;
    
    /** return whether the playlist is producing any audio output */
    bool isPlaying();

    /**
     PlaylistComponent::setColourPalette()
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
    // Component virtual methods */
    void paint (juce::Graphics&) override;
    void resized() override;
    // TableListBoxModel virtual / pure virtual methods */
    int getNumRows() override;
    void paintRowBackground(juce::Graphics &,
                            int rowNumber,
                            int width,
                            int height,
                            bool rowIsSelected) override;
    void paintCell (juce::Graphics &,
                    int rowNumber,
                    int columnId,
                    int width,
                    int height,
                    bool rowIsSelected) override;
    juce::Component* refreshComponentForCell (int rowNumber,
                                              int columnId,
                                              bool isRowSelected,
                                              juce::Component* existingComponentToUpdate
                                              ) override;
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;
    void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;
    void returnKeyPressed(int lastRowSelected) override;
    void deleteKeyPressed(int lastRowSelected) override;
    void backgroundClicked(const MouseEvent &mEv) override; // enable file load when clicking on empty playlist
    // TextEditor::Listener virtual methods */
    void textEditorTextChanged (juce::TextEditor &) override;
    // Timer pure virtual methods */
    void timerCallback() override;
    // ChangeListener pure virtual methods */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    // FileDragAndDropTarget pure virtual methods */
    bool isInterestedInFileDrag (const StringArray &files) override;
    // passes dropped files to PlaylistComponent::loadFileToMusicLib
    void filesDropped (const StringArray &files, int x, int y) override;
    
    /* ======================== */
    /* ====== properties ====== */
    /* ======================== */
    
    // juce derived properties */
    /** manage audio formats */
    juce::AudioFormatManager* formatManager;
    /** cache for waveform display */
    juce::AudioThumbnailCache thumbcache{100};
    /** table model for playlist */
    juce::TableListBox tableComponent;
    // playlist GUI element components */
    juce::Slider crossfade, crossfadeTime{juce::Slider::Rotary, juce::Slider::TextBoxLeft};
    juce::TextButton autoPlay{"auto play"}, autoCrossfade{"auto crossfade"}, loadToPlaylist{"add to playlist"}, clearPlaylist{"clear playlist"};
    juce::TextEditor searchInput;
    
    /* ===== native properties ===== */
    
    /** colour palette for use in this component's paint method */
    juce::Colour controllerBackground, controllerBody, controllerIndicator, infoTextColour, warningTextColour;
    
  /* ===== music library ===== */
    /** structure for storing tracks in music library */
    struct Track
    {
    public:
        long int        libraryId;
        juce::URL       trackURL;
        juce::String    title;
        float           length;     //in seconds
    };
    /** vector of all tracks in music library */
    std::vector<Track> musicLib;
    /** vector of tracks to be displayed in the playlist */
    std::vector<Track> tracksToDisplay;
    /** next unique library Id for insert */
    long int nextLibraryId;

    /* ===== audio players ===== */
    // pointers to audio players passed from MainComponent (assigned in constructor) */
    DJAudioPlayer* player1;
    DJAudioPlayer* player2;
    
    /* ===== GUIs for players ===== */
    // storing as a vector rather than separately simplifies some routines
    // pointers to GUIs passed from MainComponenet
    // pushed into vector in constructor
    /** vector of DeckGUIs */
    std::vector<DeckGUI*> deckGUIs;

    /* ===== general ===== */
    /** store autoplay status */
    bool autoplayToggle;
    /** store autoCrossfade status */
    bool autoCrossfadeToggle;
    /** increment for auto crossfade, controls direction (-/+) and speed */
    double autoCrossfadeInc;
    
    /** path to musicLib file - tab delineated representation of Track structs in PlaylistComponent::musicLib */
    std::string musicLibPath;
    
    /** class scope layout properties - initialised and updated in resize() */
    double rowH, colW;
    

    /* ===================== */
    /* ====== methods ====== */
    /* ===================== */
    // implement juce virtual methods

    /* ========================================= */
    /* ====== listeners and event lambdas ====== */
    /* ========================================= */
    
    /**
     PlaylistComponent::setCrossfade()
     Input                  none
     Output                 none
     Called as lambda function by onClick method of PlaylistComponent::crossfade component
     Calculates crossfade values for each of the players and passes to relevant controllers
     */
    void setCrossfade();
    
    /*
     PlaylistComponent::triggerAutoCrossfade, PlaylistComponent::engageAutoplay and
     PlaylistComponent::openFileBrowser are also called as lambdas
     but are described under 'state reporters and updaters' and 'load file and manage music library' respectively
     */
    
    /* ================================================ */
    /* ====== load file and manage music library ====== */
    /* ================================================ */
    
    /**
     PlaylistComponent::loadMusicLib()
     Input                  none
     Output                 none
     Called in constructor, opens file pointed to by musicLibPath
     Reads line by line, uses PlaylistComponent::tokeniseMusicLibLine()
     To convert line to a PlaylistComponent::Track struct
     Stores in PlaylistComponent::musicLib vector
     */
    void loadMusicLib();
    
    /**
     PlaylistComponent::saveMusicLib()
     Input                  none
     Output                 none
     When called opens file pointed to by musicLibPath
     Iterates through PlaylistComponent::musicLib
     Converts PlaylistComponent::Track struct to tab delineated string
     Writes to open file
     */
    void saveMusicLib();
    
    /**
     PlaylistComponent::openFileBrowser()
     Input                  none
     Output                 none
     Invokes juce::FileChooser for adding a music file to the library
     If successful, passes the selected file to PlaylistComponent::loadFileToMusicLib
     */
    void openFileBrowser();
    /**
     PlaylistComponent::tokeniseMusicLibLine
     Input                  std::string line
     Output                 Track struct
     @param line            a tab-delineated line from the musicLib file
     Called as a lambda by onClick method of PlaylistComponent::loadToPlaylist component
     Takes a tab-delineated line from the file pointed to by musicLibPath
     Returns a PlaylistComponent::Track struct populate with the information in the line
     */
    Track tokeniseMusicLibLine(std::string line);
    
    /**
     PlaylistComponent::loadFileToMusicLib
     Input                  juce::File
     Output                 none
     Passed File is tested as a recognised music file
     If it is, its details are extracted and added to PlaylistComponent::musicLib
     and the music library file is update
     */
    void loadFileToMusicLib(juce::File);

    /* ========================================== */
    /* ====== state reporters and updaters ====== */
    /* ========================================== */
    
    /**
     PlaylistComponent::engageAutoplay()
     Input                  none
     Output                 none
     When triggered, checks if players have audio files loaded
     If either doesn't, loads the next Track from musicLib
     Moves any loaded tracks to the end of the musicLib
     If neither player is playing, starts player 1
     */
    void engageAutoplay();
    
    /**
     PlaylistComponent::triggerAutoCrossfade
     Input                  none
     Output                 none
     When triggered, calculates crossfadeInc
     Sets autoCrossfadeToggle to true
     */
    void triggerAutoCrossfade();
    

    /* ======================= */
    /* ====== utilities ====== */
    /* ======================= */
    
    /**
     PlaylistComponent::lengthToMinutesAndSeconds
     Input                  a float representing a number of seconds
     Output                 a String of minutes and seconds with identifiers
     @param length          float - number of seconds to be converted to minutes and seconds String
    */
    juce::String lengthToMinutesAndSeconds(float length);
    
    /**
     PlaylistComponent::filterTracksToDisplayByTitle()
     Input                  std::string to search for
     Output                 none
     @param searchTerm      a std::string, usually passed from PlaylistComponent::searchInput
     Takes a string as input
     If string is empty copies PlaylistComponent::musicLib to PlaylistComponent::tracksToBeDisplayed
     Otherwise populates PlaylistComponent::tracksToBeDisplayed
     from PlaylistComponent::musicLib depending on the output of a regex match
     */
    void filterTracksToDisplayByTitle(std::string searchTerm);
    
    /**
     PlaylistComponent::loadIfNotPlaying()
     Input                  int index of track in PlaylistComponent::tracksToDisplay to load
     Output                 none
     If a player is not playing, the selected playlist track will be loaded into it
     */
    void loadIfNotPlaying(int index);
    
    /**
     PlaylistComponent::removeFromLibrary()
     Input                  long int unique library id of track
     Output                 none
     Removes the track with the given id from the library, saves and updates the display
     */
    void removeFromLibrary(long int trackId);
    
    /**
     PlaylistComponent::emptyPlaylist()
     Input                  none
     Output                 none
     Removes all tracks from the playlist
     */
    void emptyPlaylist();
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistComponent)
};
