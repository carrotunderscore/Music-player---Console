# Music-player---Console
Backend logic for the Music player (https://github.com/carrotunderscore/Music-player)

## Libraries used
FMOD (Used for playing the media)<br /> 
SFML (Used for plotting the data)<br />

## Algorithms
Phase Vocoder<br />
Fast Fourier Transform<br />

### My understanding of the Phase Vocoder algorithm (so far)
You take the PCM data (raw audio signal data) and divide that data into windows or frames. Those frames are samples of a fixed size and they overlap eachother to avoid artefacts.
The Hanning window function are then applied to the frames to reduce spectral leakage. <br /><br />
Then you apply the Short-Time Fourier Transform (STFT) to the frames to tranform them from a 
time domain to a frequency domain .This results in a complex spectrum that contains
magnitude and phase information. <br /><br />
Then you process them and in my case (pitch dilation) you change the spacing between the frames. <br /><br />
After this you do a inverse Short-Time Fourier Transform (STFT) to turn them back into a time domain. <br /><br />
When this is done you overlap the frames and sum them to reconstruct the audio signal. If you don't do this 
you can disrupt the phase continuity between frames.

