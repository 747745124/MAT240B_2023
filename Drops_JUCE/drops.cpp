#include <juce_audio_processors/juce_audio_processors.h>
#include "./drops.hpp"
using namespace juce;

struct Raindrops : public AudioProcessor
{

  AudioParameterFloat *gain;
  AudioParameterFloat *density;
  AudioParameterFloat *frequency;
  AudioParameterFloat *start;
  std::unique_ptr<Drops> drops_v1 = std::make_unique<Drops>(0.5, 10, 0.0, 4.0, 25.0, 2.0);
  std::unique_ptr<Drops> drops_v2 = std::make_unique<Drops>(0.5, 1000, 0.0, 4.0, 25.0, 2.0);
  float last_sample = drops_v1->next_sample();

  struct ValueChangeListener : public AudioProcessorParameter::Listener
  {
    ValueChangeListener(Raindrops &drops) : drops_ref(drops) {}
    // if value changed, recompute all the parameters
    void parameterValueChanged(int parameterIndex, float newValue) override
    {
      drops_ref.drops_v1->recompute(0.5, newValue, 1.5, 2.0, 25.0, 2.0);
    }
    // this one does nothing
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override
    {
      return;
    };

  private:
    Raindrops &drops_ref;
  };

  Raindrops()
      : AudioProcessor(BusesProperties()
                           .withInput("Input", AudioChannelSet::stereo())
                           .withOutput("Output", AudioChannelSet::stereo()))
  {
    addParameter(gain = new AudioParameterFloat(
                     {"gain", 1}, "Gain",
                     NormalisableRange<float>(-65, -1, 0.01f), -65));

    addParameter(density = new AudioParameterFloat(
                     {"density", 1}, "Density",
                     NormalisableRange<float>(10, 1000, 10.f), 10));

    addParameter(frequency = new AudioParameterFloat(
                     {"frequency", 1}, "Frequency",
                     NormalisableRange<float>(0.5, 440.0, 0.5f), 1.0));

    addParameter(start = new AudioParameterFloat(
                     {"start", 1}, "Start",
                     NormalisableRange<float>(0.0, 2.0, 0.1f), 1.0));

    // gain->addListener(new ValueChangeListener(*this));
  }

  /// this function handles the audio ///////////////////////////////////////
  void processBlock(AudioBuffer<float> &buffer, MidiBuffer &) override
  {
    auto left = buffer.getWritePointer(0, 0);
    auto right = buffer.getWritePointer(1, 0);
    auto amount = (1000.f - density->get()) / 1000.f;

    float sr = getSampleRate();
    auto start_sample = (int)(start->get() * sr);
    auto end_sample = (int)(1.f / frequency->get() * sr) + start_sample;

    // drops->recompute(0.5, density->get(), 1.0, 2.0, 25.0, 2.0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
      left[i] = mix(drops_v2->next_sample(start_sample, end_sample), drops_v1->next_sample(start_sample, end_sample), amount) * dbtoa(gain->get());
      left[i] = mix(left[i], last_sample, 0.5f);
      right[i] = left[i];
      last_sample = left[i];
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
    MemoryOutputStream(destData, true).writeFloat(*gain);
    MemoryOutputStream(destData, true).writeFloat(*density);
    MemoryOutputStream(destData, true).writeFloat(*frequency);
    MemoryOutputStream(destData, true).writeFloat(*start);
    /// add parameters here /////////////////////////////////////////////////
  }

  void setStateInformation(const void *data, int sizeInBytes) override
  {
    gain->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
    density->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
    frequency->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
    start->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
    /// add parameters here /////////////////////////////////////////////////
  }

  /// do not change anything below this line, probably //////////////////////

  /// general configuration /////////////////////////////////////////////////
  const String getName() const override { return "Raindrops"; }
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
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Raindrops)
};

AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
  return new Raindrops();
}