#include <SFML/Graphics.hpp>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


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

    std::vector<double> getRawAudioSignal() {
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

        FMOD_SOUND_FORMAT format;
        int channels, bits;
        sound->getFormat(nullptr, &format, &channels, &bits);

        // Get the sample rate
        sound->getDefaults(&rate, NULL);
        std::cout << "Channels: " << channels << std::endl;
        std::cout << "Bit Depth: " << bits << std::endl;
        std::cout << "Sample Rate: " << rate << " Hz" << std::endl;

        // Lock sound buffer
        void* ptr1; void* ptr2;
        unsigned int len1; unsigned int len2;
        result = sound->lock(0, len, &ptr1, &ptr2, &len1, &len2);

        short* data16 = reinterpret_cast<short*>(ptr1);
        std::vector<double> audioDataVector;  // Change type to double

        for (size_t i = 0; i < len1 / sizeof(short); i++) {
            audioDataVector.push_back(static_cast<double>(data16[i]));
        }

        // Unlock sound buffer
        result = sound->unlock(ptr1, ptr2, len1, len2);

        // Cleanup
        sound->release();
        system->close();
        system->release();

        return audioDataVector;
    }

    void segmentVectorIntoFrames(const std::vector<double>& original, std::vector<std::vector<double>>& segments, size_t secondVecSize) {
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

    template <typename T>
    std::vector<T> mergeFramesIntoVector(const std::vector<std::vector<T>>& segments, size_t secondVecSize) {
        size_t originalSize = (segments.size() - 1) * (secondVecSize / 2) + secondVecSize;
        std::vector<T> original(originalSize, 0); // Initialized with 0 for summing overlaps later

        size_t pos = 0;

        for (size_t i = 0; i < segments.size(); i++) {
            for (size_t j = 0; j < segments[i].size(); j++) {
                if (i != 0 && j < secondVecSize / 2) {
                    // Average overlapping parts
                    original[pos + j] = (original[pos + j] + segments[i][j]) / 2.0;
                }
                else {
                    original[pos + j] = segments[i][j];
                }
            }
            pos += secondVecSize / 2; // Move by half the segment size to account for overlap
        }

        return original;
    }

    // Generate a Hanning window of size N
    std::vector<double> hanningWindow(size_t N) {
        std::vector<double> window(N);

        for (size_t n = 0; n < N; n++) {
            window[n] = 0.5 * (1 - std::cos(2 * M_PI * n / (N - 1)));
        }

        return window;
    }

    std::vector<double> applyWindow(const std::vector<double>& frame, const std::vector<double>& window) {
    size_t frameSize = frame.size();

    // Check if window size matches frame size
    if (frameSize != window.size()) {
        throw std::runtime_error("Frame and window sizes must match!");
    }

    std::vector<double> windowedFrame(frameSize);

    for (size_t i = 0; i < frameSize; i++) {
        windowedFrame[i] = static_cast<short>(frame[i] * window[i]);
    }

    return windowedFrame;
}

};