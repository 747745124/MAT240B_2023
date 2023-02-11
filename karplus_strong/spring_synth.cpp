#include <juce_audio_processors/juce_audio_processors.h>
#include <stdio.h>
#include "utility.hpp"

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

using namespace juce;

// http://scp.web.elte.hu/papers/synthesis1.pdf

struct SpringSynth : public AudioProcessor
{
    AudioParameterFloat *frequency;
    AudioParameterFloat *decayTime;
    std::unique_ptr<MassSpringModel> _springModel = std::make_unique<MassSpringModel>();
    bool current_state = true;
    /// add parameters here ///////////////////////////////////////////////////
    /// add your objects here /////////////////////////////////////////////////

    SpringSynth()
        : AudioProcessor(BusesProperties()
                             .withInput("Input", AudioChannelSet::stereo())
                             .withOutput("Output", AudioChannelSet::stereo()))
    {
        addParameter(frequency = new AudioParameterFloat(
                         {"frequency", 1}, "frequency",
                         NormalisableRange<float>(20.f, 10000.f, 10.f), 440.f));
        addParameter(decayTime = new AudioParameterFloat(
                         {"delayTime", 1}, "delayTime",
                         NormalisableRange<float>(0, 10, 0.1f), 1.f));
    }

    /// this function handles the audio ///////////////////////////////////////
    void processBlock(AudioBuffer<float> &buffer, MidiBuffer &) override
    {
        // buffer.clear(0, 0, buffer.getNumSamples());
        auto left = buffer.getWritePointer(0, 0);
        auto right = buffer.getWritePointer(1, 0);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            _springModel->recalculate(frequency->get(), decayTime->get(), (float)getSampleRate());
            left[i] = right[i] = _springModel->next_sample();
        }
    }

    /// start and shutdown callbacks///////////////////////////////////////////
    void prepareToPlay(double, int) override
    {
    }
    void releaseResources() override {}

    /// maintaining persistant state on suspend ///////////////////////////////
    void getStateInformation(MemoryBlock &destData) override
    {
        // MemoryOutputStream(destData, true).writeFloat(*gain);
        /// add parameters here /////////////////////////////////////////////////
    }

    void setStateInformation(const void *data, int sizeInBytes) override
    {
        // gain->setValueNotifyingHost(
        //     MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
        //         .readFloat());

        /// add parameters here /////////////////////////////////////////////////
    }

    /// do not change anything below this line, probably //////////////////////

    /// general configuration /////////////////////////////////////////////////
    const String getName() const override { return "Quasi Band Limited"; }
    double getTailLengthSeconds() const override { return 0; }
    bool acceptsMidi() const override { return false; }
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpringSynth)
};

AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new SpringSynth();
}