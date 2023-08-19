#include "Graph.cpp"
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
#include <complex>
#include <fftw3.h>


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

std::vector<std::vector<double>> windowing(size_t frameSize, AudioPlayer audioPlayer, AudioManipulation audioManipulation, Graph graph) {

    //std::vector<double> pcmData = audioManipulation.getRawAudioSignal();
    std::vector<double> pcmData = generateSinWave(2.0, 44.1, 44100); // Need to set audioPlayer.setNumChannels(1); for playing mono audio
    audioPlayer.setNumChannels(1);
    std::vector<short> sineWaveShort = Helper<double>::convertToShort(pcmData);
    //audioPlayer.playProcessedPCMData(sineWaveShort);

    graph.plotSineWave(pcmData);

    //frame sineWays 
    std::vector<std::vector<double>> sinWaveFrames((pcmData.size() / frameSize * 2), std::vector<double>(frameSize));
    audioManipulation.segmentVectorIntoFrames(pcmData, sinWaveFrames, frameSize);

    std::vector<double> window = audioManipulation.hanningWindow(frameSize);
    //apply hanning window
    for (auto& frame : sinWaveFrames) {
        frame = audioManipulation.applyWindow(frame, window);
    }
    return sinWaveFrames;
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

//Computing FFT to a single frame
std::vector<std::complex<double>> computeFFT(const std::vector<double>& frame) {
    int N = frame.size();

    fftw_complex* in, * out;
    std::vector<std::complex<double>> result(N);

    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < N; i++) {
        in[i][0] = frame[i];
        in[i][1] = 0.0;
    }

    fftw_execute(p);

    for (int i = 0; i < N; i++) {
        result[i] = std::complex<double>(out[i][0], out[i][1]);
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return result;
}

//This calls the computeFFT on every vector in the 2D vector
void STFT(std::vector<std::vector<double>>& frames, std::vector<std::vector<std::complex<double>>>& fft_results, std::vector<std::vector<double>>& all_magnitudes, std::vector<std::vector<double>>& all_phases) {

    for (const auto& frame : frames) {
        fft_results.push_back(computeFFT(frame));
    }


    for (const auto& frame_fft : fft_results) {
        std::vector<double> magnitudes;
        std::vector<double> phases;
        for (const auto& complex_val : frame_fft) {
            double magnitude = std::abs(complex_val);
            double phase = std::arg(complex_val);

            magnitudes.push_back(magnitude);
            phases.push_back(phase);
        }
        all_magnitudes.push_back(magnitudes);
        all_phases.push_back(phases);
    }
}

std::vector<double> computeIFFT(const std::vector<std::complex<double>>& frame_fft) {
    int N = frame_fft.size();

    fftw_complex* in, * out;
    std::vector<double> result(N);

    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

    for (int i = 0; i < N; i++) {
        in[i][0] = frame_fft[i].real();
        in[i][1] = frame_fft[i].imag();
    }

    fftw_execute(p);

    for (int i = 0; i < N; i++) {
        result[i] = out[i][0] / N;
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return result;
}

//This calls the computeIFFT on every vector in the 2D vector
void inverseSTFT(std::vector<std::vector<double>>& time_frames, const std::vector<std::vector<std::complex<double>>>& stft_results) {
    for (const auto& frame_fft : stft_results) {
        std::vector<double> time_domain_frame = computeIFFT(frame_fft);
        time_frames.push_back(time_domain_frame);
    }

}

std::vector<std::vector<std::complex<double>>> timeStretch(const std::vector<std::vector<std::complex<double>>>& spectra, double alpha, int sampleRate) {
    int fftSize = spectra[0].size();
    int hopSize = fftSize / 2;  // Assuming a 50% overlap for analysis
    int stretchHopSize = int(hopSize / alpha);

    std::vector<double> previousPhase(fftSize, 0.0);
    std::vector<double> phaseAccumulation(fftSize, 0.0);

    std::vector<std::vector<std::complex<double>>> stretchedSpectraFrames;

    for (size_t frame = 0; frame < spectra.size(); ++frame) {
        std::vector<std::complex<double>> currentSpectrum = spectra[frame];
        std::vector<std::complex<double>> outputSpectrum(fftSize, 0.0);

        for (int i = 0; i < fftSize; ++i) {
            double binFrequency = (double)i * sampleRate / fftSize;
            double expectedPhaseDiff = 2.0 * M_PI * binFrequency * hopSize / sampleRate;

            double currentPhase = std::arg(currentSpectrum[i]);
            double phaseDiff = currentPhase - previousPhase[i] - expectedPhaseDiff;

            // Phase unwrapping
            phaseDiff -= 2.0 * M_PI * round(phaseDiff / (2.0 * M_PI));
            double trueFrequency = binFrequency + phaseDiff * sampleRate / (2.0 * M_PI * hopSize);

            phaseAccumulation[i] += stretchHopSize * trueFrequency * 2.0 * M_PI / sampleRate;

            outputSpectrum[i] = std::abs(currentSpectrum[i]) * std::complex<double>(cos(phaseAccumulation[i]), sin(phaseAccumulation[i]));

            previousPhase[i] = currentPhase;
        }

        stretchedSpectraFrames.push_back(outputSpectrum);
    }

    return stretchedSpectraFrames;
}

int main()
{
    AudioManipulation audioManipulation("C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3");
    AudioPlayer audioPlayer("C:\\Users\\rober\\Music\\\sound_file_formats_testing\\sample.mp3");
    Graph graph;
    size_t frameSize = 1024;

    std::vector<std::vector<double>> frames = windowing(frameSize, audioPlayer, audioManipulation, graph);
    std::vector<std::vector<std::complex<double>>> fft_results;
    std::vector<std::vector<double>> all_magnitudes;
    std::vector<std::vector<double>> all_phases;
    std::vector<std::vector<double>> time_frames;


    STFT(frames, fft_results, all_magnitudes, all_phases);
   
    //Here should the time strecthing occur
   // Assuming you've already computed the STFT and have the resulting spectra stored in a variable called 'spectra'
    double alpha = 1.5;  // This means the audio will be stretched by a factor of 1.5
    int sampleRate = 44100;  // For example, assuming a common sample rate of 44.1 kHz

    std::vector<std::vector<std::complex<double>>> stretchedSpectra = timeStretch(fft_results, alpha, sampleRate);

    inverseSTFT(time_frames, stretchedSpectra);

    //Overlap-Add:
    std::vector<double> frames2 = audioManipulation.reconstructSignal(time_frames, frameSize);
    graph.plotSineWave(frames2);
    std::vector<short> shortFrames = Helper<double>::convertToShort(frames2);
    audioPlayer.setNumChannels(1);
    //audioPlayer.playProcessedPCMData(shortFrames);
    

    int hej = 0;

    
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