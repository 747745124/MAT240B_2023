#include <iostream>
#include <cmath>
#define pi 3.1415926

void basic_fm(float freq, float sample_rate, int sample_frame)
{
    // variables and constants
    float osc;              // output of the saw oscillator
    float osc2;             // output of the saw oscillator 2
    float phase;            // phase accumulator
    float w;                // normalized frequency
    float scaling;          // scaling amount
    float DC;               // DC compensation
    float *output;          // pointer to array of floats
    float pw;               // pulse width of the pulse, 0..1
    float norm;             // normalization amount
    float const a0 = 2.5f;  // precalculated coeffs
    float const a1 = -1.5f; // for HF compensation
    float in_hist;          // delay for the HF filter
    // calculate w and scaling
    w = freq / sample_rate; // normalized frequency
    float n = 0.5f - w;
    scaling = 13.0f * n * n * n * n; // calculate scaling
    DC = 0.376f - w * 0.752f;        // calculate DC compensation
    osc = 0.f;
    phase = 0.f;            // reset oscillator and phase
    norm = 1.0f - 2.0f * w; // calculate normalization
    // process loop for creating a bandlimited saw wave
    while (--sample_frame >= 0)
    {
        // increment accumulator
        phase += 2.0f * w;
        if (phase >= 1.0f)
            phase -= 2.0f;
        // calculate next sample
        osc = (osc + sin(2 * pi * (phase + osc * scaling))) * 0.5f;
        // compensate HF rolloff
        float out = a0 * osc + a1 * in_hist;
        in_hist = osc;
        out = out + DC;         // compensate DC offset
        *output++ = out * norm; // store normalized result
    }
    // process loop for creating a bandlimited PWM pulse
    while (--sample_frame >= 0)
    {
        // increment accumulator
        phase += 2.0f * w;
        if (phase >= 1.0f)
            phase -= 2.0f;
        // calculate saw1
        osc = (osc + sin(2 * pi * (phase + osc * scaling))) * 0.5f;
        // calculate saw2
        osc2 = (osc2 + sin(2 * pi * (phase + osc2 * scaling + pw))) * 0.5f;
        float out = osc - osc2; // subtract two saw waves
        // compensate HF rolloff
        out = a0 * out + a1 * in_hist;
        in_hist = out;
        *output++ = out * norm; // store normalized result
    }
}

int main()
{
    basic_fm(440.f, 44100.f, 100);
    return 0;
}