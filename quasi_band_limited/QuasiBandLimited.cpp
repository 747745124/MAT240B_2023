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

struct QuasiBandLimited : public AudioProcessor
{
  AudioParameterFloat *gain;
  AudioParameterFloat *note;
  AudioParameterFloat *depth;
  AudioParameterFloat *mod;
  std::unique_ptr<Cycle> cycle = std::make_unique<Cycle>();
  std::unique_ptr<Cycle> modulator = std::make_unique<Cycle>();
  /// add parameters here ///////////////////////////////////////////////////
  /// add your objects here /////////////////////////////////////////////////

  QuasiBandLimited()
      : AudioProcessor(BusesProperties()
                           .withInput("Input", AudioChannelSet::stereo())
                           .withOutput("Output", AudioChannelSet::stereo()))
  {
    addParameter(gain = new AudioParameterFloat(
                     {"gain", 1}, "Gain",
                     NormalisableRange<float>(-65, -1, 0.01f), -65));
    /// add parameters here /////////////////////////////////////////////////
    addParameter(note = new AudioParameterFloat(
                     {"note", 1}, "Note",
                     NormalisableRange<float>(-2, 129, 0.01f), 40));
    addParameter(mod = new AudioParameterFloat(
                     {"mod", 1}, "Mod",
                     NormalisableRange<float>(-27, 100, 0.01f), -27));
    addParameter(depth = new AudioParameterFloat(
                     {"depth", 1}, "Depth",
                     NormalisableRange<float>(0, 127, 0.01f), 0));
  }

  /// this function handles the audio ///////////////////////////////////////
  void processBlock(AudioBuffer<float> &buffer, MidiBuffer &) override
  {
    /// put your own code here instead of this code /////////////////////////
    // buffer.clear(0, 0, buffer.getNumSamples());
    // buffer.clear(1, 0, buffer.getNumSamples());
    auto left = buffer.getWritePointer(0, 0);
    auto right = buffer.getWritePointer(1, 0);
    // left[0] = right[0] = dbtoa(gain->get()); // click!

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
      // the reason to do this, is becuase sin is calculated numerically (likely)
      // it may not be a periodic function

      float A = dbtoa(gain->get());
      float alpha = cycle->next_sample(mtof(note->get()));
      float beta = mtof(mod->get());
      float index = dbtoa(depth->get());

      auto res = A * cycle->next_sample(alpha + index * soft_clip(modulator->next_sample(beta)));
      left[i] = right[i] = res;
    }
  }

  /// start and shutdown callbacks///////////////////////////////////////////
  void prepareToPlay(double, int) override {}
  void releaseResources() override {}

  /// maintaining persistant state on suspend ///////////////////////////////
  void getStateInformation(MemoryBlock &destData) override
  {
    MemoryOutputStream(destData, true).writeFloat(*gain);
    /// add parameters here /////////////////////////////////////////////////
  }

  void setStateInformation(const void *data, int sizeInBytes) override
  {
    gain->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
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
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuasiBandLimited)
};

AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
  return new QuasiBandLimited();
}