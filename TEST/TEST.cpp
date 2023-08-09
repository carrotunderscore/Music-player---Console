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
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



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

template<typename T>
int drawRawAudioSignalPlot(const std::vector<T>& pcmData) {
    // 1. Initialize an SFML window
    sf::RenderWindow window(sf::VideoMode(2500, 600), "Line Plot using std::vector");
    window.setFramerateLimit(60);

    // 2. Populate your vector
    std::vector<T> data = pcmData;

    // Define axes for reference
    sf::RectangleShape xAxis(sf::Vector2f(2500, 2));
    xAxis.setPosition(10, 300);
    xAxis.setFillColor(sf::Color::Black);

    sf::RectangleShape yAxis(sf::Vector2f(2, 580));
    yAxis.setPosition(400, 10);
    yAxis.setFillColor(sf::Color::Black);

    // Find the range for normalization and scaling
    T maxY = *std::max_element(data.begin(), data.end());
    T minY = *std::min_element(data.begin(), data.end());

    float scaleX = 2500.0f / (data.size() - 1);
    float scaleY = 290.0f / std::max(std::abs(static_cast<float>(maxY)), std::abs(static_cast<float>(minY)));

    float pixelsPerSecond = 2500.0f / 3.474f; // 1180 pixels for 3.474 seconds

    sf::Font font;
    if (!font.loadFromFile("Montserrat-Black.otf")) {
        std::cerr << "Font not found!" << std::endl;
        return -1;
    }
    sf::Text startLabel("0", font, 15);

    float audioDuration = 3.474; // Length of your audio in seconds
    sf::Text endLabel(std::to_string(audioDuration) + "s", font, 15);
    endLabel.setPosition(780, 310); // Just below the end of x-axis
    endLabel.setFillColor(sf::Color::Black);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear(sf::Color::White);

        // Draw the 0.1 second increment labels
        for (float t = 0; t <= 3.474f; t += 0.1f) {
            float xPosition = 10 + t * pixelsPerSecond;

            sf::Text label;
            label.setFont(font);
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(1) << t;
            label.setString(stream.str());
            label.setCharacterSize(12); // Adjust as needed
            label.setFillColor(sf::Color::Black);

            label.setPosition(xPosition, 560);
            window.draw(label);
        }

        // Draw axes
        window.draw(xAxis);
        window.draw(yAxis);

        // 4. Draw the line
        for (size_t i = 0; i < data.size() - 1; i++) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(10 + i * scaleX, 300 - data[i] * scaleY), sf::Color::Blue),
                sf::Vertex(sf::Vector2f(10 + (i + 1) * scaleX, 300 - data[i + 1] * scaleY), sf::Color::Blue)
            };

            // 5. Render the line
            window.draw(line, 2, sf::Lines);
        }

        window.display();
    }
}

template<typename T>
int drawRawAudioSignalPlot(const std::vector<std::vector<T>>& pcmDataMatrix) {
    // 1. Initialize an SFML window
    sf::RenderWindow window(sf::VideoMode(2500, 600), "Line Plot using std::vector");
    window.setFramerateLimit(60);

    // Colors for the lines
    std::vector<sf::Color> colors = { sf::Color::Yellow, sf::Color::Blue, sf::Color::Green, sf::Color::Red };

    // Define axes for reference
    sf::RectangleShape xAxis(sf::Vector2f(2500, 2));
    xAxis.setPosition(10, 300);
    xAxis.setFillColor(sf::Color::Black);

    sf::RectangleShape yAxis(sf::Vector2f(2, 580));
    yAxis.setPosition(400, 10);
    yAxis.setFillColor(sf::Color::Black);

    sf::Font font;
    if (!font.loadFromFile("Montserrat-Black.otf")) {
        std::cerr << "Font not found!" << std::endl;
        return -1;
    }

    sf::Text startLabel("0", font, 15);
    float audioDuration = 3.474;
    sf::Text endLabel(std::to_string(audioDuration) + "s", font, 15);
    endLabel.setPosition(780, 310);
    endLabel.setFillColor(sf::Color::Black);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear(sf::Color::White);

        //... Other codes for drawing x-axis, y-axis, and labels remain unchanged

        // Now, loop over the 2D vector and plot each 1D vector
        for (size_t vecIndex = 0; vecIndex < pcmDataMatrix.size(); ++vecIndex) {
            const auto& data = pcmDataMatrix[vecIndex];

            // Find the range for normalization and scaling
            T maxY = *std::max_element(data.begin(), data.end());
            T minY = *std::min_element(data.begin(), data.end());
            float scaleX = 2500.0f / (data.size() - 1);
            float scaleY = 290.0f / std::max(std::abs(static_cast<float>(maxY)), std::abs(static_cast<float>(minY)));

            // Select color based on index
            sf::Color currentColor = colors[vecIndex % colors.size()];

            // Draw the line for current data
            for (size_t i = 0; i < data.size() - 1; i++) {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(10 + i * scaleX, 300 - data[i] * scaleY), currentColor),
                    sf::Vertex(sf::Vector2f(10 + (i + 1) * scaleX, 300 - data[i + 1] * scaleY), currentColor)
                };

                window.draw(line, 2, sf::Lines);
            }
        }

        window.display();
    }
    return 0;
}

void plotFrame(const std::vector<short>& frame, sf::RenderWindow& window) {
    float scaleX = window.getSize().x / static_cast<float>(frame.size());
    float scaleY = window.getSize().y / 65536.0f;  // assuming 16-bit PCM

    for (size_t i = 0; i < frame.size() - 1; i++) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(i * scaleX, window.getSize().y / 2 - frame[i] * scaleY), sf::Color::Blue),
            sf::Vertex(sf::Vector2f((i + 1) * scaleX, window.getSize().y / 2 - frame[i + 1] * scaleY), sf::Color::Blue)
        };
        window.draw(line, 2, sf::Lines);
    }
}

void displayFrames(const std::vector<std::vector<short>>& frames) {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Time-Domain Plot of Individual Frames");
    size_t currentFrameIndex = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Right && currentFrameIndex < frames.size() - 1) {
                    currentFrameIndex++;
                }
                if (event.key.code == sf::Keyboard::Left && currentFrameIndex > 0) {
                    currentFrameIndex--;
                }
            }
        }

        window.clear();
        plotFrame(frames[currentFrameIndex], window);
        window.display();
    }
}

void drawLinesFromData(const std::vector<std::vector<short>>& data, sf::RenderWindow& window) {
    float spacing = 20.0f;  // spacing between points
    float verticalSpacing = 15.0f;  // spacing between each inner vector's set of lines
    float verticalLineWidth = 2.0f;  // width of the vertical line

    float x = 0.0f;

    for (const auto& innerVector : data) {
        if (!innerVector.empty()) {
            // Draw lines based on data in the inner vector
            for (size_t i = 0; i < innerVector.size() - 1; ++i) {
                sf::VertexArray line(sf::Lines, 2);
                line[0].position = sf::Vector2f(x + i * spacing, innerVector[i]);
                line[1].position = sf::Vector2f(x + (i + 1) * spacing, innerVector[i + 1]);
                window.draw(line);
            }

            x += innerVector.size() * spacing;

            // Draw vertical line after each inner vector
            sf::VertexArray verticalLine(sf::Lines, 2);
            verticalLine[0].position = sf::Vector2f(x, 0);
            verticalLine[1].position = sf::Vector2f(x, window.getSize().y);  // till the height of the window
            verticalLine[0].color = sf::Color::Red;  // Setting color to red for clarity
            verticalLine[1].color = sf::Color::Red;
            window.draw(verticalLine);

            x += verticalSpacing + verticalLineWidth;
        }
    }
}

// Generate a Hanning window of size N
std::vector<double> hanningWindow(size_t N) {
    std::vector<double> window(N);

    for (size_t n = 0; n < N; n++) {
        window[n] = 0.5 * (1 - std::cos(2 * M_PI * n / (N - 1)));
    }

    return window;
}

// Apply window to multiple frames of audio data
std::vector<std::vector<double>> applyWindow(const std::vector<std::vector<short>>& frames, const std::vector<double>& window) {
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

int main()
{
    const char* homer = "C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3";
    const char* huey = "C:\\Users\\rober\\Music\\Huey Lewis and The News - Greatest Hits Japanese Singles Collection (2023) Mp3 320kbps [PMEDIA]\\11 - Hip To Be Square.mp3";
   
    AudioPlayer audio(homer, 44100, 2 );
    //audio.playSong();

    AudioManipulation audioManipulation(homer);
    std::vector<short> pcmData = audioManipulation.getRawAudioSignal();


    size_t secondVectorSize = 512;
    std::vector<std::vector<short>> pcmDataFrames((pcmData.size() / secondVectorSize * 2), std::vector<short>(secondVectorSize));
    audioManipulation.segmentVectorIntoFrames(pcmData, pcmDataFrames, secondVectorSize);



    for (size_t i = 0; i < pcmDataFrames[100].size(); i++) {

        std::cout << pcmDataFrames[100][i] << ", ";
    }
    std::vector<double> window = hanningWindow(pcmData.size());  // Assuming all frames are of the same size
    std::vector<std::vector<double>> windowedFrames = applyWindow(pcmDataFrames, window);

    std::cout << "------------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < windowedFrames[100].size(); i++) {

        std::cout << windowedFrames[100][i] << ", ";
    }
    //std::vector<double> window = hanningWindow(segments[0].size());  // Assuming all frames are of the same size
    //std::vector<std::vector<double>> windowedFrames = applyWindow(segments, window);



    drawRawAudioSignalPlot(pcmDataFrames[100]);
    //drawRawAudioSignalPlot(windowedFrames[100]);
    
    int hej = 0;







    //drawRawAudioSignalPlot(pcmData);

   

    
    
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