/*
* TIMER
    auto start_time = std::chrono::high_resolution_clock::now(); //Start timer
    PUT CODE TO TIME HERE!!!!!
    auto end_time = std::chrono::high_resolution_clock::now(); // End timer
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Elapsed time: " << elapsed_time.count() << " ms" << std::endl;

    FUNCTIONS TO PLAY CERTAIN AUDIOFILES, HOMER, HUEY AND A GENERATED SINEWAVE

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



*/

