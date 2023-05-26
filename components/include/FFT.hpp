#include "esphome.h"
#include <arduinoFFT.h>

arduinoFFT FFT = arduinoFFT();

std::vector<double> GetFrequencyDomain(const std::vector<uint8_t> &data)
{
    const uint8_t amplitude = 150;
    std::vector<double> vReal;
    std::vector<double> vImag;
    int bands[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (uint16_t i = 0; i < data.size(); i++) {
        vReal.push_back(data[i] << 8);
        vImag.push_back(0.0); //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
    }

    FFT.Windowing(&vReal[0], data.size(), FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(&vReal[0], &vImag[0], data.size(), FFT_FORWARD);
    FFT.ComplexToMagnitude(&vReal[0], &vImag[0], data.size());
    for (int i = 0; i < 8; i++) {
        bands[i] = 0;
    }
  
    for (int i = 2; i < (data.size()/2); i++){ // Don't use sample 0 and only first SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
        if (vReal[i] > 2000) { // Add a crude noise filter, 10 x amplitude or more
        if (i<=2 )             bands[0] = max(bands[0], (int)(vReal[i]/amplitude)); // 125Hz
        if (i >3   && i<=5 )   bands[1] = max(bands[1], (int)(vReal[i]/amplitude)); // 250Hz
        if (i >5   && i<=7 )   bands[2] = max(bands[2], (int)(vReal[i]/amplitude)); // 500Hz
        if (i >7   && i<=15 )  bands[3] = max(bands[3], (int)(vReal[i]/amplitude)); // 1000Hz
        if (i >15  && i<=30 )  bands[4] = max(bands[4], (int)(vReal[i]/amplitude)); // 2000Hz
        if (i >30  && i<=53 )  bands[5] = max(bands[5], (int)(vReal[i]/amplitude)); // 4000Hz
        if (i >53  && i<=200 ) bands[6] = max(bands[6], (int)(vReal[i]/amplitude)); // 8000Hz
        if (i >200           ) bands[7] = max(bands[7], (int)(vReal[i]/amplitude)); // 16000Hz
        }
    }

    std::vector<double> result;
    for (int i = 0; i < 8; i++) {
        result.push_back(bands[i]);
    }
    return result;
}

double GetMaxFrequency(const std::vector<uint8_t> &data, uint8_t amplitude = 50, uint16_t sampling_frequency = 16000)
{
    std::vector<double> vReal;
    std::vector<double> vImag;
    for (uint16_t i = 0; i < data.size(); i++)
    {
        vReal.push_back(data[i] << 8);
        vImag.push_back(0.0);
    }

    FFT.Windowing(&vReal[0], data.size(), FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(&vReal[0], &vImag[0], data.size(), FFT_FORWARD);
    FFT.ComplexToMagnitude(&vReal[0], &vImag[0], data.size());

    double maxAmplitude = 0;
    int maxIndex = 0;
    for (int i = 2; i < (data.size() / 2); i++)
    {
        if (vReal[i] > maxAmplitude && vReal[i] > amplitude)
        {
            maxAmplitude = vReal[i];
            maxIndex = i;
        }
    }

    double frequency = maxIndex * (sampling_frequency / data.size());
    return frequency;
}

std::string ScaleNameFromFrequency(double frequency)
{
    double frequencies[] = {
        16.35, 17.32, 18.35, 19.45, 20.6, 21.83, 23.12, 24.5, 25.96, 27.5, 29.14, 30.87,
        32.7, 34.65, 36.71, 38.89, 41.2, 43.65, 46.25, 49, 51.91, 55, 58.27, 61.74,
        65.41, 69.3, 73.42, 77.78, 82.41, 87.31, 92.5, 98, 103.83, 110, 116.54, 123.47,
        130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185, 196, 207.65, 220, 233.08, 246.94, 
        261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392, 415.3, 440, 466.16, 493.88, 
        523.25, 554.37, 587.33, 622.25, 659.25, 698.46, 739.99, 783.99, 830.61, 880, 932.33, 987.77, 
        1046.5, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 1760, 1864.66, 1975.53, 
        2093, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520, 3729.31, 3951.07,
        4186.01, 4434.92, 4698.64, 4978.03, 5274.04, 5587.65, 5919.91, 6271.93, 6644.88, 7040, 7458.62, 7902.13
    };

    std::string scaleNames[] = {
        "C0", "C#0/Db0", "D0", "D#0/Eb0", "E0", "F0", "F#0/Gb0", "G0", "G#0/Ab0", "A0", "A#0/Bb0", "B0",
        "C1", "C#1/Db1", "D1", "D#1/Eb1", "E1", "F1", "F#1/Gb1", "G1", "G#1/Ab1", "A1", "A#1/Bb1", "B1",
        "C2", "C#2/Db2", "D2", "D#2/Eb2", "E2", "F2", "F#2/Gb2", "G2", "G#2/Ab2", "A2", "A#2/Bb2", "B2",
        "C3", "C#3/Db3", "D3", "D#3/Eb3", "E3", "F3", "F#3/Gb3", "G3", "G#3/Ab3", "A3", "A#3/Bb3", "B3",
        "C4", "C#4/Db4", "D4", "D#4/Eb4", "E4", "F4", "F#4/Gb4", "G4", "G#4/Ab4", "A4", "A#4/Bb4", "B4",
        "C5", "C#5/Db5", "D5", "D#5/Eb5", "E5", "F5", "F#5/Gb5", "G5", "G#5/Ab5", "A5", "A#5/Bb5", "B5",
        "C6", "C#6/Db6", "D6", "D#6/Eb6", "E6", "F6", "F#6/Gb6", "G6", "G#6/Ab6", "A6", "A#6/Bb6", "B6",
        "C7", "C#7/Db7", "D7", "D#7/Eb7", "E7", "F7", "F#7/Gb7", "G7", "G#7/Ab7", "A7", "A#7/Bb7", "B7",
        "C8", "C#8/Db8", "D8", "D#8/Eb8", "E8", "F8", "F#8/Gb8", "G8", "G#8/Ab8", "A8", "A#8/Bb8", "B8"
    };

    int closestIndex = 0;
    double minDistance = abs(frequency - frequencies[0]);
    for (int i = 1; i < sizeof(frequencies) / sizeof(frequencies[0]); i++)
    {
        double distance = abs(frequency - frequencies[i]);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestIndex = i;
        }
    }
    return scaleNames[closestIndex];
}