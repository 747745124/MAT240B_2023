// Karl Yerkes
// 2023-01-17
// MAT 240B ~ Audio Programming
// Assignment 2 ~ Quasi Band-Limited Sawtooth and Pulse Generator
//

#include <juce_audio_processors/juce_audio_processors.h>
#include "QuasiFM.hpp"
#include "utility.hpp"

using namespace juce;

// http://scp.web.elte.hu/papers/synthesis1.pdf

struct QuasiBandImpulse : public AudioProcessor
{
    AudioParameterFloat *note;
    AudioParameterFloat *scale;
    AudioParameterBool *mode;
    std::unique_ptr<QuasiImpulse> _qimp = std::make_unique<QuasiImpulse>();
    std::unique_ptr<QuasiSaw> _qsaw = std::make_unique<QuasiSaw>();

    /// add parameters here ///////////////////////////////////////////////////
    /// add your objects here /////////////////////////////////////////////////

    QuasiBandImpulse()
        : AudioProcessor(BusesProperties()
                             .withInput("Input", AudioChannelSet::stereo())
                             .withOutput("Output", AudioChannelSet::stereo()))
    {
        addParameter(note = new AudioParameterFloat(
                         {"note", 1}, "note",
                         NormalisableRange<float>(-2, 129, 0.01f), 40));
        addParameter(scale = new AudioParameterFloat(
                         {"scale", 1}, "scale",
                         NormalisableRange<float>(-10, 10, 0.1f), 1));
        addParameter(mode = new AudioParameterBool({"mode", 2}, "mode", false));
    }

    /// this function handles the audio ///////////////////////////////////////
    void processBlock(AudioBuffer<float> &buffer, MidiBuffer &) override
    {

        auto left = buffer.getWritePointer(0, 0);
        auto right = buffer.getWritePointer(1, 0);
        _qimp->configure(mtof(note->get()));
        _qsaw->configure(mtof(note->get()));

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            // the reason to do this, is becuase sin is calculated numerically (likely)
            // it may not be a periodic function
            if (mode->get())
                left[i] = right[i] = soft_clip(scale->get() * _qimp->next_sample());
            else
                left[i] = right[i] = soft_clip(scale->get() * _qsaw->next_sample());
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuasiBandImpulse)
};

AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new QuasiBandImpulse();
}