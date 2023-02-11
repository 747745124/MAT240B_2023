// Karl Yerkes
// 2023-01-30
// MAT 240B ~ Audio Programming
// Assignment 3 ~ Karplus-Strong string modeling
//

#include <juce_audio_processors/juce_audio_processors.h>
#include "utility.hpp"

class DelayLine : std::vector<float>
{
    //
    int index = 0;

public:
    float read(float seconds_ago, float samplerate)
    {
        //
        jassert(seconds_ago < size() / samplerate);

        float i = index - seconds_ago * samplerate;
        if (i < 0)
        {
            i += size();
        }
        return at((int)i); // no linear interpolation
    }

    void write(float value)
    {
        jassert(size() > 0);
        at(index) = value; // overwrite the oldest value

        // handle the wrapping for circular buffer
        index++;
        if (index >= size())
            index = 0;
    }

    void allocate(float seconds, float samplerate)
    {
        // floor(seconds * samplerate) + 1 samples
        resize((int)floor(seconds * samplerate) + 1);
    }
};

// https://en.wikipedia.org/wiki/Harmonic_oscillator
struct MassSpringModel
{
    // this the whole state of the simulation
    //
    float position{0}; // m
    float velocity{0}; // m/s

    // These are cached properties of the model; They govern the behaviour. We
    // recalculate them given frequency, decay time, and playback rate.
    //
    float springConstant{0};     // N/m
    float dampingCoefficient{0}; // N·s/m

    void show()
    {
        printf("position:%f velocity:%f springConstant:%f dampingCoefficient:%f\n",
               position, velocity, springConstant, dampingCoefficient);
    }

    void reset()
    {
        // show();
        position = velocity = 0;
    }

    float next_sample()
    {
        // This is semi-implicit Euler integration with time-step 1. The
        // playback rate is "baked into" the constants. Spring force and damping
        // force are accumulated into velocity. We let mass is 1, so it
        // disappears. Velocity is accumulated into position which is
        // interpreted as oscillator amplitude.
        //
        float acceleration = 0;

        acceleration = -position * springConstant - dampingCoefficient * velocity;

        // XXX put code here

        velocity += acceleration;
        position += velocity;

        /*
            printf("position:%f velocity:%f springConstant:%f
           dampingCoefficient: %f\n", position, velocity, springConstant,
           dampingCoefficient);
        */
        return position;
    }

    float operator()()
    {
        return next_sample();
    }

    // Use these to measure the kinetic, potential, and total energy of the
    // system.
    float ke() { return velocity * velocity / 2; }
    float pe() { return position * position * springConstant / 2; }
    float te() { return ke() + pe(); }

    // "Kick" the mass-spring system such that we get a nice (-1, 1) oscillation.
    //
    void trigger()
    {
        // We want the "mass" to move in (-1, 1). What is the potential energy
        // of a mass-spring system at 1? PE == k * x * x / 2 == k / 2. So, we
        // want a system with k / 2 energy, but we don't want to just set the
        // displacement to 1 because that would make a click. Instead, we want
        // to set the velocity. What velocity would we need to have energy k /
        // 2? KE == m * v * v / 2 == k / 2. or v * v == k. so...
        //
        velocity += sqrt(springConstant);

        // XXX put code here

        // How might we improve on this? Consider triggering at a level
        // depending on frequency according to the Fletcher-Munson curves.
    }

    void recalculate(float frequency, float decayTime, float playbackRate)
    {

        // frequency equals to 2pi/(sqrt(1-ksi^2)*w0)
        //  sqrt(1-ksi^2) = sqrt(1-c^2/4k)
        //  2pi/ (sqrt(1-c^2) / 2)
        // freq = 4pi/(sqrt(1-c^2) )
        // sqrt(1-c^2) =4pi/freq
        // c^2 = 1- 16pi^2/freq^2
        // operations.
        dampingCoefficient = 2 / (decayTime * playbackRate);
        springConstant = pow(frequency * M_PI * 2 / playbackRate, 2) +
                         1 / pow(decayTime * playbackRate, 2);
        trigger();

        // sample rate is "baked into" these constants to save on per-sample
        // operations.
    }
};

struct BooleanOscillator
{
    float value = 1;
    float increment = 0;
    void frequency(float hertz, float samplerate)
    {
        assert(hertz >= 0);
        increment = hertz / samplerate;
    }
    void period(float hertz, float samplerate)
    {
        frequency(1 / hertz, samplerate);
    }
    bool operator()()
    {
        value += increment;
        bool b = value >= 1;
        value = wrap(value, 1.0f, 0.0f);
        return b;
    }
};

struct KarplusStrongModel
{
    float gain = 1;
    float t60 = 1;
    float delayTime = 1;
    int sampleRate = 48000;

    DelayLine delay;
    MeanFilter filter;

    void configure(float hertz, float seconds, float samplerate)
    {
        delayTime = 1 / hertz;
        t60 = seconds;
        int k = t60 / delayTime;
        gain = pow(dbtoa(-60.0), 1.0f / k);
        sampleRate = samplerate;
        delay.allocate(seconds, samplerate);
        // given t60 (`seconds`) and frequency (`Hertz`), calculate
        // the gain...
        //
        // for a given frequency, our algorithm applies *gain*
        // frequency-many times per second. given a t60 time we can
        // calculate how many times (n)  gain will be applied in
        // those t60 seconds. we want to reduce the signal by 60dB
        // over t60 seconds or over n-many applications. this means
        // that we want gain to be a number that, when multiplied
        // by itself n times, becomes 60 dB quieter than it began.
        //
        // the size of the delay *is* the period of the vibration
        // of the string, so 1/period = frequency.
    }

    float trigger()
    {
        // fill the delay line with noise
        int n = int(ceil(delayTime * sampleRate));
        for (int i = 0; i < n; ++i)
        {
            delay.write(gain * (double)std::rand() / (RAND_MAX));
        }
    }

    float operator()()
    {
        float v = filter(delay.read(delayTime, sampleRate)) * gain;
        delay.write(v);
        return v;
    }
    // XXX put code here
};

using namespace juce;

class KarplusStrong : public AudioProcessor
{
    AudioParameterFloat *gain;
    AudioParameterFloat *note;
    AudioParameterFloat *time;
    AudioParameterFloat *freq;
    BooleanOscillator timer;
    MassSpringModel string;
    KarplusStrongModel karplus;
    /// add parameters here ///////////////////////////////////////////////////

public:
    KarplusStrong()
        : AudioProcessor(BusesProperties()
                             .withInput("Input", AudioChannelSet::stereo())
                             .withOutput("Output", AudioChannelSet::stereo()))
    {
        addParameter(gain = new AudioParameterFloat(
                         {"gain", 1}, "Gain",
                         NormalisableRange<float>(-65, -1, 0.01f), -65));
        addParameter(
            note = new AudioParameterFloat(
                {"note", 1}, "Note", NormalisableRange<float>(-2, 129, 0.01f), 40));
        addParameter(
            time = new AudioParameterFloat(
                {"decaytime", 1}, "decayTime", NormalisableRange<float>(0.1, 10.0, 0.1f), 1.0));
        addParameter(
            freq = new AudioParameterFloat(
                {"playback frequency", 1}, "playback frequency", NormalisableRange<float>(0.1, 10.0, 0.1f), 1.0));
        /// add parameters here /////////////////////////////////////////////

        // XXX juce::getSampleRate() is not valid here
    }

    float previous = 0;

    /// handling the actual audio! ////////////////////////////////////////////
    void processBlock(AudioBuffer<float> &buffer, MidiBuffer &) override
    {
        buffer.clear(0, 0, buffer.getNumSamples());
        auto left = buffer.getWritePointer(0, 0);
        auto right = buffer.getWritePointer(1, 0);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {

            if (timer())
            {
                timer.period(freq->get(), (float)getSampleRate());
                karplus.configure(mtof(note->get()), time->get(), (float)getSampleRate());
                karplus.trigger();
            }

            left[i] = right[i] = karplus() * dbtoa(gain->get());
            ;
        }
    }

    /// handle doubles ? //////////////////////////////////////////////////////
    // void processBlock(AudioBuffer<double>& buffer, MidiBuffer&) override {
    //   buffer.applyGain(dbtoa((float)*gain));
    // }

    /// start and shutdown callbacks///////////////////////////////////////////
    void prepareToPlay(double, int) override
    {
        // XXX when does this get called? seems to not get called in stand-alone
    }
    void releaseResources() override {}

    /// maintaining persistant state on suspend ///////////////////////////////
    void getStateInformation(MemoryBlock &destData) override
    {
        MemoryOutputStream(destData, true).writeFloat(*gain);
    }

    void setStateInformation(const void *data, int sizeInBytes) override
    {
        gain->setValueNotifyingHost(
            MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
                .readFloat());
    }

    /// general configuration /////////////////////////////////////////////////
    const String getName() const override { return "KS Pluck"; }
    double getTailLengthSeconds() const override { return 0; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }

    /// for handling presets //////////////////////////////////////////////////
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const String getProgramName(int) override { return "None"; }
    void changeProgramName(int, const String &) override {}

    /// ?????? ////////////////////////////////////////////////////////////////
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override
    {
        const auto &mainInLayout = layouts.getChannelSet(true, 0);
        const auto &mainOutLayout = layouts.getChannelSet(false, 0);

        return (mainInLayout == mainOutLayout && (!mainInLayout.isDisabled()));
    }

    /// automagic user interface //////////////////////////////////////////////
    AudioProcessorEditor *createEditor() override
    {
        return new GenericAudioProcessorEditor(*this);
    }
    bool hasEditor() const override { return true; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KarplusStrong)
};

AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new KarplusStrong();
}