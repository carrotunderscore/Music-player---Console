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
#include "AudioPlayer.cpp"
#include "AudioManipulation.cpp"
#include <cmath>
#include <numeric>  // for std::accumulate
#include "Helper.cpp"


//Function for testing purposes
bool writeToTextFile(std::string text, bool success) {
    std::ofstream file("C:\\Users\\rober\\Music\\example.txt", std::ios::app);

    if (file.is_open()) {
        if (success) {
            file << text << " succeeded" << std::endl;
        }
        else {
            file << text << " failed" << std::endl;

        }
        
        file.close();
        return true;
    }
    else {
        std::cout << "Unable to open file";
        return false;
    }

}

std::vector<double> generateSinWave(double duration, double frequency, int sampleRate) {
    size_t N = sampleRate * duration;  // Total number of samples

    std::vector<double> data(N);

    for (size_t n = 0; n < N; n++) {
        // Scale the sine wave to fit within 16-bit range (-32768 to 32767)
        data[n] = 32767 * std::sin(2 * M_PI * frequency * n / sampleRate);
    }

    return data;
}

void plotSineWave(std::vector<double> data) {

    sf::RenderWindow window(sf::VideoMode(2500, 600), "Sine Wave Plot");
    window.setFramerateLimit(60);

    // Scaling
    float scaleX = static_cast<float>(window.getSize().x) / (data.size() - 1);
    float scaleY = window.getSize().y / 100000.0f;  // Decreasing the scale for a more "zoomed out" look

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear(sf::Color::White);

        for (size_t i = 0; i < data.size() - 1; i++) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(i * scaleX, window.getSize().y / 2 - data[i] * scaleY), sf::Color::Blue),
                sf::Vertex(sf::Vector2f((i + 1) * scaleX, window.getSize().y / 2 - data[i + 1] * scaleY), sf::Color::Blue)
            };

            window.draw(line, 2, sf::Lines);
        }

        window.display();
    }
}

void playSineWaveAndProcess() {
    /*
    size_t frameSize = 100;

    //Class initialization
    AudioPlayer audio("C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3", 44100, 2);
    AudioManipulation audioManipulation("C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3");

    //generate sineWave -DONE
    std::vector<double> sineWave = generateSinWave(3.0, 440, 44100);
    //std::vector<short> sineWave = generateSinWave(1.0, 5, 100);
    // plot the generated sine wave
    plotSineWave(sineWave);  // Plot 1 second of a 44 Hz sine wave

    //frame sineWays 
    std::vector<std::vector<double>> sinWaveFrames((sineWave.size() / frameSize * 2), std::vector<double>(frameSize));
    audioManipulation.segmentVectorIntoFrames(sineWave, sinWaveFrames, frameSize);

    std::vector<double> window = audioManipulation.hanningWindow(frameSize);
    //apply hanning window
    for (auto& frame : sinWaveFrames) {
        frame = audioManipulation.applyWindow(frame, window);
    }
    // reverse framing
    std::vector<double> reversedSinWaveFrames = audioManipulation.mergeFramesIntoVector(sinWaveFrames, frameSize);
    // plot the reversed framing
    plotSineWave(reversedSinWaveFrames);



    std::vector<short> sineWaveShort = convertToShort(sineWave);
    std::vector<short> reversedSinWaveFramesShort = convertToShort(reversedSinWaveFrames);
    audio.playProcessedPCMData(sineWaveShort);
    audio.playProcessedPCMData(reversedSinWaveFramesShort);
    */
}

void windowing(size_t frameSize) {

    AudioPlayer audioPlayer("C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3");
    AudioManipulation audioManipulation("C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3");


    std::vector<double> pcmData = audioManipulation.getRawAudioSignal();
    std::vector<double> pcmData2 = generateSinWave(2.0, 440.0, 44100); // Need to set audioPlayer.setNumChannels(1); for playing mono audio

    std::vector<short> sineWaveShort = Helper<double>::convertToShort(pcmData);
    audioPlayer.playProcessedPCMData(sineWaveShort);

    plotSineWave(pcmData);

    //frame sineWays 
    std::vector<std::vector<double>> sinWaveFrames((pcmData.size() / frameSize * 2), std::vector<double>(frameSize));
    audioManipulation.segmentVectorIntoFrames(pcmData, sinWaveFrames, frameSize);

    std::vector<double> window = audioManipulation.hanningWindow(frameSize);
    //apply hanning window
    for (auto& frame : sinWaveFrames) {
        frame = audioManipulation.applyWindow(frame, window);
    }
    /*
    * // TODO: This reverse framing is gonna be implemented after the processing has been done.
    // reverse framing
    std::vector<double> reversedSinWaveFrames = audioManipulation.mergeFramesIntoVector(sinWaveFrames, frameSize);
    // plot the reversed framing
    plotSineWave(reversedSinWaveFrames);
    std::vector<short> reversedSinWaveFramesShort = Helper<double>::convertToShort(reversedSinWaveFrames);
    audioPlayer.playProcessedPCMData(reversedSinWaveFramesShort);
    */
}

int main()
{
    windowing(512);
    
}





















/*
AudioPlayer = play/pause, next song, before song, speed up, slow down

AudioManipulation = Pitch dilation

loop through 2d vector
 for (size_t i = 0; i < segments.size(); i++) {
        std::cout << i << ": ";
        for (size_t x = 0; x < segments[i].size(); x++) {
            std::cout << segments[i][x] << ", "; // Using a tab character for spacing
        }
        std::cout << std::endl; // New line after each row
    }









*/

/*
    * .mp3 = FMOD_MPEGSEARCH
    .flac, .mid, .aiff = FMOD_CREATESAMPLE
    .aac
    .ac3
    .amr
    .au
    .mka
    .ogg borde funka
    .ra
    .voc
    .wav borde funka
    .wma
    */