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
    float sampleRate;        // Sample rate of the audio file (e.g., 44100 Hz)
    int numChannels;       // Number of audio channels (1 for mono, 2 for stereo)
    std::vector<short> audioData;  // The actual audio data
    std::string fileExtension;
    float* audioLengthInMs;
    FMOD_RESULT result;
    FMOD::System* system;
    FMOD::Sound* sound;
    FMOD_CREATESOUNDEXINFO exinfo;
    unsigned int position;
    bool isLooping = false;
    unsigned int beforeLoopPosition = 0;
    unsigned int loopStart = 0;
    unsigned int loopEnd = 0;
    bool isPaused;


public:
    // Constructor
    AudioPlayer(){
        system = NULL;
        sound = NULL;
        result = FMOD::System_Create(&system);      // Create the main system object.
        result = system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.

        memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.filebuffersize = 1024 * 16;        /* Increase the default file chunk size to handle seeking inside large playlist files that may be over 2kb. */
        
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

    void play(FMOD::Channel* channel) {
        channel->setPaused(false); // actually start playing the channel
        isPaused = false; // update the state
        std::cout << "PLAYING" << std::endl;
    }
    void pause(FMOD::Channel* channel) {
        channel->setPaused(true); // actually pause the channel
        isPaused = true; // update the state
        std::cout << "PAUSED" << std::endl;

    }
    void skipForward500ms(FMOD::Channel* channel) {
        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
        position += 500;
        channel->setPosition(position, FMOD_TIMEUNIT_MS);

    }
    void skipForward100ms(FMOD::Channel* channel) {
        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
        position += 100;
        channel->setPosition(position, FMOD_TIMEUNIT_MS);
    }
    void skipBackwards500ms(FMOD::Channel* channel) {
        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
        position -= 500;
        channel->setPosition(position, FMOD_TIMEUNIT_MS);
    }
    void skipBackwards100ms(FMOD::Channel* channel) {
        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
        position -= 100;
        channel->setPosition(position, FMOD_TIMEUNIT_MS);
    }
    void turnLoopOn(FMOD::Channel* channel) {
        sound->setMode(FMOD_LOOP_NORMAL); // Enable looping
        isLooping = true;
        std::cout << "Looping is now enabled" << std::endl;
        channel->stop(); // Stop the sound from playing
        sound->release(); // Release the FMOD::Sound object
        beforeLoopPosition = position;
    }
    void turnLoopOff(FMOD::Channel* channel) {
        sound->setMode(FMOD_LOOP_OFF); // Disable looping
        isLooping = false;
        std::cout << "Looping is now disabled" << std::endl;
        channel->stop(); // Stop the sound from playing
        sound->release(); // Release the FMOD::Sound object
        beforeLoopPosition = position;
    }
    void setStartLoopPosition(FMOD::Channel* channel) {
        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
        loopStart = position;
    }
    void setEndLoopPosition(FMOD::Channel* channel) {
        channel->getPosition(&position, FMOD_TIMEUNIT_MS);
        loopEnd = position;
    }

    bool SetupSound(const char* filePath) {

        // Open the sound in a mode that's light and suited for information gathering
        result = system->createSound(filePath, FMOD_OPENONLY, nullptr, &sound);

        if (result != FMOD_OK) {
            // Error handling goes here
            return false;
        }

        // Query the sound's properties
        FMOD_SOUND_FORMAT format;
        int bits;
        unsigned int length;
        sound->getFormat(nullptr, &format, &numChannels, &bits);
        length = sound->getLength(&length, FMOD_TIMEUNIT_PCMBYTES);

        // Since you might want the file extension:
        fileExtension = std::filesystem::path(filePath).extension().string();

        // Extract sample rate
        sound->getDefaults(&sampleRate, nullptr);

        // Release the temp sound now that we have its properties
        sound->release();

        // Prepare the FMOD_CREATESOUNDEXINFO struct based on the retrieved properties
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.defaultfrequency = sampleRate;
        exinfo.numchannels = numChannels;
        exinfo.format = format;
        exinfo.length = length;

        // Now create the sound ready for playback
        result = system->createSound(filePath, FMOD_DEFAULT, &exinfo, &sound);

        return (result == FMOD_OK);
    }

    // Function to set up sound from PCM data
    bool SetupSound(const std::vector<short>& pcmData) {
        FMOD::Sound* tempSound;

        // Open the sound in a mode that's light and suited for information gathering
        result = system->createSound(filePath, FMOD_OPENONLY, nullptr, &tempSound);

        if (result != FMOD_OK) {
            // Error handling goes here
            return false;
        }

        // Query the sound's properties
        FMOD_SOUND_FORMAT format;
        int bits;
        unsigned int length;
        tempSound->getFormat(nullptr, &format, &numChannels, &bits);
        length = tempSound->getLength(&length, FMOD_TIMEUNIT_PCMBYTES);

        // Since you might want the file extension:
        fileExtension = std::filesystem::path(filePath).extension().string();

        // Extract sample rate
        tempSound->getDefaults(&sampleRate, nullptr);

        // Release the temp sound now that we have its properties
        tempSound->release();

        // Prepare the FMOD_CREATESOUNDEXINFO struct for PCM data
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.length = pcmData.size() * sizeof(short);
        exinfo.defaultfrequency = sampleRate; // Assuming you know the sample rate for PCM data
        exinfo.numchannels = numChannels;     // Assuming you know the channel count for PCM data
        exinfo.format = FMOD_SOUND_FORMAT_PCM16;

        // Create sound from PCM data
        result = system->createSound(
            reinterpret_cast<const char*>(pcmData.data()),
            FMOD_OPENMEMORY | FMOD_OPENRAW,
            &exinfo,
            &sound
        );

        return (result == FMOD_OK);
    }

    bool playSong(const char* filePath) {
        bool isPlaying = false;
        
        setFileExtension(filePath);
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
                    float frequency;
                    switch (ch) {
                    case ' ':
                    case 'k':
                        if (isPaused) {
                            play(channel);
                            break;
                        }
                        if (!isPaused) {
                            pause(channel);
                            break;
                        }
                        break;

                    case 'l':
                        skipForward500ms(channel);
                        break;

                    case 'j':
                        skipBackwards500ms(channel);
                        break;

                    case 'o':
                        skipForward100ms(channel);
                        break;

                    case 'i':
                        skipBackwards100ms(channel);
                        break;

                    case 'e':
                        if (isLooping) {
                            turnLoopOff(channel);
                        }
                        else {
                            turnLoopOn(channel);
                        }
                        break;

                    case 'q':
                        setStartLoopPosition(channel);
                        break;

                    case 'w':
                        setEndLoopPosition(channel);
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

    bool playProcessedPCMData(const std::vector<short>& pcmData) {
        
        FMOD::Channel* channel = nullptr;

        FMOD_RESULT result;

        // Initialize FMOD system
        result = FMOD::System_Create(&system);
        if (result != FMOD_OK) {
            std::cerr << "FMOD error: " << FMOD_ErrorString(result) << std::endl;
            return false;
        }

        result = system->init(512, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK) {
            std::cerr << "FMOD error: " << FMOD_ErrorString(result) << std::endl;
            system->release();
            return false;
        }

        // Set up the extra info for FMOD
        FMOD_CREATESOUNDEXINFO exinfo = {};
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.length = pcmData.size() * sizeof(short); // size in bytes
        exinfo.defaultfrequency = 44100; // Assuming a sample rate of 44.1kHz
        exinfo.numchannels = 2;  // Mono audio
        exinfo.format = FMOD_SOUND_FORMAT_PCM16; // 16-bit PCM data

        // Create a sound from the PCM data
        result = system->createSound(
            reinterpret_cast<const char*>(pcmData.data()),
            FMOD_OPENMEMORY_POINT | FMOD_OPENRAW,
            &exinfo,
            &sound
        );


        // Get the format details:
        FMOD_SOUND_FORMAT format;
        int channels, bits;
        sound->getFormat(nullptr, &format, &channels, &bits);

        std::cout << "Format: " << format << std::endl;   // The format (e.g., FMOD_SOUND_FORMAT_PCM16)
        std::cout << "Channels: " << channels << std::endl;  // 1 for mono, 2 for stereo, etc.
        std::cout << "Bit Depth: " << bits << std::endl;     // 8, 16, 24, 32, etc.

        // Get the sample rate:
        float rate;
        sound->getDefaults(&rate, nullptr);
        std::cout << "Sample Rate: " << rate << " Hz" << std::endl;

        // Length in samples:
        unsigned int length;
        sound->getLength(&length, FMOD_TIMEUNIT_PCM);
        std::cout << "Length: " << length << " samples" << std::endl;

        if (result != FMOD_OK) {
            std::cerr << "Error creating sound from PCM data: " << FMOD_ErrorString(result) << std::endl;
            system->close();
            system->release();
            return false;
        }

        // Play the sound
        result = system->playSound(sound, nullptr, false, &channel);

        if (result != FMOD_OK) {
            std::cerr << "Error playing sound: " << FMOD_ErrorString(result) << std::endl;
            sound->release();
            system->close();
            system->release();
            return false;
        }

        // Wait for sound to finish playing (simple check)
        bool isPlaying = true;
        while (isPlaying) {
            channel->isPlaying(&isPlaying);
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep for a short while to prevent high CPU usage
        }

        // Cleanup
        sound->release();
        system->close();
        system->release();

        return true;
    }


};