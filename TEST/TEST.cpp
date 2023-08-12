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

void changePosition(FMOD::Channel* channel, int changeInMilliseconds) {
    unsigned int currentPosition = 0;
    channel->getPosition(&currentPosition, FMOD_TIMEUNIT_MS);

    unsigned int newPosition = currentPosition + changeInMilliseconds;
    channel->setPosition(newPosition, FMOD_TIMEUNIT_MS);
}

// Apply window to multiple frames of audio data
std::vector<std::vector<double>> applyWindow(const std::vector<std::vector<double>>& frames, const std::vector<double>& window) {
    size_t numFrames = frames.size();

    size_t frameSize = frames[0].size(); // Assuming all frames have the same size
    
    std::vector<std::vector<double>> windowedFrames(numFrames, std::vector<double>(frameSize));

    for (size_t j = 0; j < numFrames; j++) {
        for (size_t i = 0; i < frameSize; i++) {
            
            //std::cout << i << std::endl;
            windowedFrames[j][i] = frames[j][i] * window[i];
        }
    }

    return windowedFrames;
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

std::vector<double> applyHanningWindow(const std::vector<double>& data, const std::vector<double>& window) {
    std::vector<double> windowedData(data.size());
    std::transform(data.begin(), data.end(), window.begin(), windowedData.begin(), std::multiplies<double>());
    return windowedData;
}


std::vector<short> convertToShort(const std::vector<double>& input) {
    std::vector<short> output;
    output.reserve(input.size());

    for (double val : input) {
        // Clip the values to fit into short
        val = std::max(-32768.0, std::min(32767.0, val));

        // Round to nearest integer and convert to short
        output.push_back(static_cast<short>(std::round(val)));
    }

    return output;
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

void playHomer() {
    const char* song2 = "C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3";
    AudioPlayer audioPlayer(song2);
    AudioManipulation audioManipulation(song2);

    std::vector<double> pcmData = audioManipulation.getRawAudioSignal();
    std::vector<short> sineWaveShort = convertToShort(pcmData);
    audioPlayer.playProcessedPCMData(sineWaveShort);
}

void playSineWave() {
    AudioPlayer audioPlayer;
    std::vector<double> pcmData2 = generateSinWave(2.0, 440.0, 44100);
    std::vector<short> sineWaveShort2 = convertToShort(pcmData2);
    audioPlayer.setNumChannels(1); //Görs manuellt just nu pga test syften
    audioPlayer.playProcessedPCMData(sineWaveShort2);
}
void playHuey() {
    const char* song = "C:\\Users\\rober\\Music\\Huey Lewis and The News - Greatest Hits Japanese Singles Collection (2023) Mp3 320kbps [PMEDIA]\\11 - Hip To Be Square.mp3";
    AudioPlayer audioPlayer(song);
    audioPlayer.loadSong(song);
    AudioManipulation audioManipulation(song);

    std::vector<double> pcmData = audioManipulation.getRawAudioSignal();
    std::vector<short> sineWaveShort3 = convertToShort(pcmData);
    audioPlayer.playProcessedPCMData(sineWaveShort3);
}

int main()
{
    size_t frameSize = 512;
    /*
    auto start_time = std::chrono::high_resolution_clock::now(); //Start timer
    PUT CODE TO TIME HERE!!!!!
    auto end_time = std::chrono::high_resolution_clock::now(); // End timer
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Elapsed time: " << elapsed_time.count() << " ms" << std::endl;
    */

    AudioPlayer audioPlayer("C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3");
    AudioManipulation audioManipulation("C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3");
    std::vector<double> pcmData2 = audioManipulation.getRawAudioSignal();
    std::vector<double> pcmData = generateSinWave(1.0, 44, 44100);

    std::vector<short> sineWaveShort = convertToShort(pcmData);
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

    // reverse framing
    std::vector<double> reversedSinWaveFrames = audioManipulation.mergeFramesIntoVector(sinWaveFrames, frameSize);
    // plot the reversed framing
    plotSineWave(reversedSinWaveFrames);


    std::vector<short> reversedSinWaveFramesShort = convertToShort(reversedSinWaveFrames);
    audioPlayer.playProcessedPCMData(reversedSinWaveFramesShort);
    
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