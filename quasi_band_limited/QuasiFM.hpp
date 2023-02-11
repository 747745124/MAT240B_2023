#pragma once
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

#define pi 3.1415926

class QuasiFM
{
protected:
    // variables and constants
    float osc = 0.f;           // output of the saw oscillator
    float osc2 = 0.f;          // output of the saw oscillator 2
    float phase = 0.f;         // phase accumulator
    float w = 0.f;             // normalized frequency
    float scaling = 0.f;       // scaling amount
    float DC = 0.f;            // DC compensation
    std::vector<float> result; // results
    float pw = 0.5;            // pulse width of the pulse, 0..1
    float norm = 0.f;          // normalization amount
    float const a0 = 2.5f;     // precalculated coeffs
    float const a1 = -1.5f;    // for HF compensation
    float in_hist = 0.f;       // delay for the HF filter
    float virtual_filter_param = 0.9f;

public:
    // calculate w and scaling

    void configure(float freq, float sample_rate = 44100)
    {
        w = freq / sample_rate; // normalized frequency
        float n = 0.5f - w;
        scaling = virtual_filter_param * 13.0f * n * n * n * n; // calculate scaling
        DC = 0.376f - w * 0.752f;                               // calculate DC compensation
        // osc = 0.f;
        // phase = 0.f;            // reset oscillator and phase
        norm = 1.0f - 2.0f * w; // calculate normalization
    }

    virtual float next_sample() = 0;

    void clear_result()
    {
        result.clear();
    }

    void write_to_file(std::string filename)
    {
        std::ofstream _file;
        _file.open(filename);
        for (const auto &out : result)
        {
            _file << out << std::endl;
        }

        _file.close();
    }

    float operator()()
    {
        return next_sample();
    }
};

class QuasiImpulse : public QuasiFM
{
public:
    QuasiImpulse() : QuasiFM{} {};

    float next_sample() override
    {
        // process loop for creating a bandlimited PWM pulse
        // increment accumulator
        phase += 2.0f * w;
        if (phase >= 1.0f)
            phase -= 2.0f;
        // calculate saw1
        osc = (osc + sin(1.f * pi * (phase + osc * scaling))) * 0.5;
        // calculate saw2
        osc2 = (osc2 + sin(1.f * pi * (phase + osc2 * scaling + pw))) * 0.5;
        float out = osc - osc2; // subtract two saw waves
        // compensate HF rolloff
        out = a0 * out + a1 * in_hist;
        in_hist = osc - osc2;         // input history
        result.push_back(out * norm); // store normalized result
        return out * norm;
    }
};

class QuasiSaw : public QuasiFM
{
public:
    QuasiSaw() : QuasiFM{} {};

    float next_sample() override
    {
        // increment accumulator
        phase += 2.0f * w;
        if (phase >= 1.0f)
            phase -= 2.0f;
        // calculate next sample
        osc = (osc + sin(2 * pi * (phase + osc * scaling))) * 0.5;
        // compensate HF rolloff
        float out = a0 * osc + a1 * in_hist;
        in_hist = osc;
        out = out + DC;
        result.push_back(out * norm);
        return out * norm;
    }
};