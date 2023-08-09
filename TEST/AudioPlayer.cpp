#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <filesystem>
#include <fstream>
#include <conio.h>  // For _getch()
#include <windows.h>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include <sstream>
#include <iomanip>

class AudioPlayer {


private:
    const char* filePath;  // File path of the audio file
    int sampleRate;        // Sample rate of the audio file (e.g., 44100 Hz)
    int numChannels;       // Number of audio channels (1 for mono, 2 for stereo)
    std::vector<short> audioData;  // The actual audio data
    std::string fileExtension;
    float* audioLengthInMs;
    FMOD_RESULT result;
    FMOD::System* system;
    FMOD::Sound* sound;
    FMOD_CREATESOUNDEXINFO exinfo;

public:
    // Constructor
    AudioPlayer(const char* filePath, int sampleRate, int numChannels) :
        filePath(filePath), sampleRate(sampleRate), numChannels(numChannels) {
        system = NULL;
        sound = NULL;
        result = FMOD::System_Create(&system);      // Create the main system object.
        result = system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.

        memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.filebuffersize = 1024 * 16;        /* Increase the default file chunk size to handle seeking inside large playlist files that may be over 2kb. */
        
        setFileExtension(filePath);
        getAudioLength(sound, audioLengthInMs);
        
    }

    // Getters
    std::string getFilePath() const { return filePath; }
    int getSampleRate() const { return sampleRate; }
    int getNumChannels() const { return numChannels; }
    float* getAudioLengthInMs() const { return audioLengthInMs; }
    const std::vector<short>& getAudioData() const { return audioData; }
    std::string getFileExtension() {
        return fileExtension;
    }

    // Setters
    void setFilePath(const char* filePath) { this->filePath = filePath; }
    void setSampleRate(int sampleRate) { this->sampleRate = sampleRate; }
    void setNumChannels(int numChannels) { this->numChannels = numChannels; }
    FMOD_RESULT  getAudioLength(FMOD::Sound* sound, float* lengthInSeconds) {
        if (!sound || !lengthInSeconds) {
            return FMOD_ERR_INVALID_PARAM; // Check for null pointers
        }
        unsigned int lengthMS;
        FMOD_RESULT result = sound->getLength(&lengthMS, FMOD_TIMEUNIT_MS);
        *lengthInSeconds = lengthMS / 1000.0f;
        float defaultFrequency;
        sound->getDefaults(&defaultFrequency, nullptr);
        std::cout << "Length of the audio file: " << *lengthInSeconds << " seconds" << std::endl;
        audioLengthInMs = lengthInSeconds;
        return result; // Return the FMOD_RESULT to allow error handling

    }
    void setAudioData(const std::vector<short>& audioData) { this->audioData = audioData; }
    void setFileExtension(const char* file_path) {
        std::filesystem::path p(file_path);
        std::cout << p.extension().string() << std::endl;
        fileExtension = p.extension().string();
    }

    void ClearScreen()
    {
        COORD cursorPosition;
        cursorPosition.X = 0;
        cursorPosition.Y = 0;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
    }

    std::vector<int> convertPositionToMinSec(int position) {
        int seconds_total = position / 1000;
        int minutes = seconds_total / 60;
        int seconds = seconds_total % 60;
        int milliSeconds = position % 1000;

        return { minutes, seconds, milliSeconds };
    }

    bool playSong() {
        unsigned int position;
        bool isPlaying = false;
        unsigned int loopStart = 0;
        unsigned int loopEnd = 0;
        bool isLooping = false;
        unsigned int beforeLoopPosition = 0;

        while (true) {
            FMOD::Sound* sound = NULL;
            std::string extension = getFileExtension();
            if (extension == ".mp3") {
                result = system->createSound(filePath, FMOD_MPEGSEARCH | FMOD_NONBLOCKING, &exinfo, &sound);
            }
            else if (extension == ".wav") {
                result = system->createSound(filePath, FMOD_CREATESTREAM | FMOD_NONBLOCKING, &exinfo, &sound);
            }
            else {
                result = system->createSound(filePath, FMOD_NONBLOCKING | FMOD_NONBLOCKING, &exinfo, &sound);
            }
            if (result != FMOD_OK) {
                std::cout << "Failed to load sound: " << FMOD_ErrorString(result) << std::endl;
            }

            FMOD_OPENSTATE openState;
            do {
                sound->getOpenState(&openState, NULL, NULL, NULL);

            } while (openState != FMOD_OPENSTATE_READY);

            if (isLooping) {
                sound->setMode(FMOD_LOOP_NORMAL); // Enable looping
            }
            else {
                sound->setMode(FMOD_LOOP_OFF);
            }

            FMOD::Channel* channel = NULL;
            system->playSound(sound, NULL, false, &channel);
            if (beforeLoopPosition != 0) {
                channel->setPosition(beforeLoopPosition, FMOD_TIMEUNIT_MS);
            }
            // Check if the sound is playing
            do
            {
                //Loops the song if loop is enabled
                channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                if (isLooping && position >= loopEnd) {
                    // If we're beyond the loop end, reset position to loop start
                    channel->setPosition(loopStart, FMOD_TIMEUNIT_MS);
                }
                ClearScreen();
                //PROGRESS in minutes and seconds
                channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                int minute = convertPositionToMinSec(position)[0];
                int second = convertPositionToMinSec(position)[1];
                int milliSecond = convertPositionToMinSec(position)[2];

                std::cout << "Current position: " << minute << " min " << second << " sec " << milliSecond << " ms" << std::endl;
                std::cout << "Controls:\nSpace/K = PLAY/PAUSE\nL = forward 500 ms\nJ = backward 500 ms\nO = forward 100 ms\nI = backward 100 ms\nToggle loop = E\nSet loopstart = Q\nSet loopend = W\n" << std::endl;
                if (loopStart != 0) {
                    std::cout << "Loop start: " << convertPositionToMinSec(loopStart)[0] << " min " << convertPositionToMinSec(loopStart)[1] << " sec " << convertPositionToMinSec(loopStart)[2] << " ms" << std::endl;
                    std::cout << "Loop end: " << convertPositionToMinSec(loopEnd)[0] << " min " << convertPositionToMinSec(loopEnd)[1] << " sec " << convertPositionToMinSec(loopEnd)[2] << " ms" << std::endl;
                }

                if (_kbhit()) {
                    char ch = _getch();
                    bool isPaused;
                    float frequency;

                    switch (ch) {
                    case ' ':
                    case 'k':
                        channel->getPaused(&isPaused);
                        channel->setPaused(!isPaused);
                        if (isPaused) {
                            std::cout << "PLAYING" << std::endl;
                        }
                        if (!isPaused) {
                            std::cout << "PAUSED" << std::endl;
                        }
                        break;

                    case 'l':
                        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                        position += 500;
                        channel->setPosition(position, FMOD_TIMEUNIT_MS);
                        break;

                    case 'j':
                        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                        position -= 500;
                        channel->setPosition(position, FMOD_TIMEUNIT_MS);
                        break;

                    case 'o':
                        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                        position += 100;
                        channel->setPosition(position, FMOD_TIMEUNIT_MS);
                        break;

                    case 'i':
                        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                        position -= 100;
                        channel->setPosition(position, FMOD_TIMEUNIT_MS);
                        break;

                    case 'e':
                        if (isLooping) {
                            sound->setMode(FMOD_LOOP_OFF); // Disable looping
                            isLooping = false;
                            std::cout << "Looping is now disabled" << std::endl;
                            channel->stop(); // Stop the sound from playing
                            sound->release(); // Release the FMOD::Sound object
                            beforeLoopPosition = position;
                        }
                        else {
                            sound->setMode(FMOD_LOOP_NORMAL); // Enable looping
                            isLooping = true;
                            std::cout << "Looping is now enabled" << std::endl;
                            channel->stop(); // Stop the sound from playing
                            sound->release(); // Release the FMOD::Sound object
                            beforeLoopPosition = position;
                        }
                        break;

                    case 'q':
                        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                        loopStart = position;
                        break;

                    case 'w':
                        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
                        loopEnd = position;
                        break;

                    case 's':
                        channel->getFrequency(&frequency); // Get the current frequency
                        frequency *= 1.1f; // Increase the frequency to speed up the song
                        channel->setFrequency(frequency); // Apply the new frequency
                        std::cout << "Speed up song" << std::endl;
                        break;
                    }
                }
                system->update();

                channel->isPlaying(&isPlaying);
                if (!isPlaying)
                    break;
            } while (isPlaying);
        }
        return true;
    }

};