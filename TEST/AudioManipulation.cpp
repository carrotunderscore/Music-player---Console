#include <SFML/Graphics.hpp>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <iostream>



class AudioManipulation {

private:
    const char* filePath;  // File path of the audio file
    FMOD_RESULT result;
    FMOD::System* system;
    FMOD::Sound* sound;
    FMOD_CREATESOUNDEXINFO exinfo;

public:
    //Constructor
    AudioManipulation(const char* filePath) : filePath(filePath) {
        system = NULL;
        sound = NULL;
        result = FMOD::System_Create(&system);      // Create the main system object.
        result = system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.

        memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.filebuffersize = 1024 * 16;        /* Increase the default file chunk size to handle seeking inside large playlist files that may be over 2kb. */
    }

    // Getters
    std::string getFilePath() const { return filePath; }

    // Setters

    std::vector<short> getRawAudioSignal() {
        FMOD_SOUND_TYPE type;
        unsigned int len;
        float rate;

        FMOD::System* system;
        FMOD::Sound* sound;
        FMOD_RESULT result;

        // Initialize FMOD system
        result = FMOD::System_Create(&system);
        result = system->init(512, FMOD_INIT_NORMAL, nullptr);


        // Load audio file
        result = system->createSound(filePath, FMOD_DEFAULT, nullptr, &sound);

        // Get length and format
        result = sound->getLength(&len, FMOD_TIMEUNIT_PCMBYTES);

        float audioDurationSeconds;
        //FMOD_RESULT resultLength = getAudioLength(sound, &audioDurationSeconds);


        FMOD_SOUND_FORMAT format;
        int channels, bits;
        sound->getFormat(nullptr, &format, &channels, &bits);

        // Get the sample rate
        sound->getDefaults(&rate, NULL);
        std::cout << "Channels: " << channels << std::endl;  // 1 for mono, 2 for stereo, etc.
        std::cout << "Bit Depth: " << bits << std::endl;     // 8, 16, 24, 32, etc.
        std::cout << "Sample Rate: " << rate << " Hz" << std::endl;



        // Lock sound buffer
        void* ptr1; void* ptr2;
        unsigned int len1; unsigned int len2;
        result = sound->lock(0, len, &ptr1, &ptr2, &len1, &len2);

        short* data16 = reinterpret_cast<short*>(ptr1);
        std::vector<short> audioDataVector;

        for (size_t i = 0; i < len1 / sizeof(short); i++) {
            audioDataVector.push_back(data16[i]);
        }
        /*
        for (unsigned int i = 0; i < len1 / 2; i++) { // divide by 2 because 2 bytes per 16-bit sample
            std::cout << data16[i] << " ";
        }
        */

        // Unlock sound buffer
        result = sound->unlock(ptr1, ptr2, len1, len2);

        // Cleanup
        sound->release();
        system->close();
        system->release();

        return audioDataVector;
    }

    void segmentVectorIntoFrames(const std::vector<short>& original, std::vector<std::vector<short>>& segments, size_t secondVecSize) {
        if (secondVecSize == 0) {
            std::cerr << "Error: secondVecSize cannot be 0." << std::endl;
            return;
        }
        size_t seg = secondVecSize;
        size_t expectedOuterSize = original.size() / (secondVecSize / 2);

        // Ensure outer vector is of correct size
        if (segments.size() != expectedOuterSize) {
            segments.resize(expectedOuterSize);
        }

        for (size_t i = 0; i < expectedOuterSize; i++) {
            // Ensure inner vector is of correct size
            if (segments[i].size() != secondVecSize) {
                segments[i].resize(secondVecSize);
            }

            int num = 0;
            //std::cout << i << ", ";
            for (size_t x = seg - secondVecSize; x < seg; x++) {
                if (x < original.size()) {
                    segments[i][num] = original[x];
                }
                num++;
            }
            seg += (secondVecSize / 2);
        }
    }


};