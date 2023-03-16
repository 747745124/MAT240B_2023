#include <juce_audio_processors/juce_audio_processors.h>
#include "./drops.hpp"
#include "./simplified_drops.hpp"
#include <mutex>
#include <thread>

using namespace juce;
std::mutex m;

struct Raindrops : public AudioProcessor
{

  AudioParameterFloat *gain;
  AudioParameterFloat *randomness;
  AudioParameterFloat *overall_frequency;
  AudioParameterFloat *decay_frequency;
  AudioParameterFloat *start;
  AudioParameterFloat *density;
  std::unique_ptr<Drops> drops = std::make_unique<Drops>(0.5, 10, 0.0, 4.0, 25.0, 2.0);
  std::unique_ptr<Drops_Simplified> drops_simplified = std::make_unique<Drops_Simplified>(0.5, 100, 25.0, 2.0);
  uint time_counter = 0;
  float running_max = -1e3;

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

    addParameter(randomness = new AudioParameterFloat(
                     {"randomness", 1}, "Randomness",
                     NormalisableRange<float>(0.0, 1.0, 0.1f), 0.5));

    addParameter(overall_frequency = new AudioParameterFloat(
                     {"frequency", 1}, "Frequency",
                     NormalisableRange<float>(0.5, 440.0, 0.5f), 1.0));

    addParameter(decay_frequency = new AudioParameterFloat(
                     {"decay_frequency", 1}, "Decay Frequency",
                     NormalisableRange<float>(1.0, 100.0, 1.f), 25.));

    addParameter(start = new AudioParameterFloat(
                     {"start", 1}, "Start",
                     NormalisableRange<float>(0.0, 2.0, 0.1f), 1.0));
  }

  /// this function handles the audio ///////////////////////////////////////
  void processBlock(AudioBuffer<float> &buffer, MidiBuffer &) override
  {
    auto left = buffer.getWritePointer(0, 0);
    auto right = buffer.getWritePointer(1, 0);

    drops_simplified = std::make_unique<Drops_Simplified>(randomness->get(), density->get(), decay_frequency->get());
    float sr = getSampleRate();

    auto start_time = start->get();
    auto end_time = 1.f / overall_frequency->get() + start_time;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
      if (time_counter >= end_time)
      {
        time_counter = start_time;
      }

      float res = 0.0f;
      for (int i = 0; i < drops_simplified->drops.size(); i++)
      {
        res += drops_simplified->drops[i].sample_at_simplified(time_counter);
      }

      if (fabs(res) > fabs(running_max))
      {
        running_max = res;
      }

      left[i] = res * dbtoa(gain->get()) / fabs(running_max);
      right[i] = left[i];

      time_counter += 1.0 / sr;
    }
  }

  /// start and shutdown callbacks///////////////////////////////////////////
  void prepareToPlay(double, int) override
  {
  }
  void releaseResources() override
  {
  }

  /// maintaining persistant state on suspend ///////////////////////////////
  void getStateInformation(MemoryBlock &destData) override
  {
    MemoryOutputStream(destData, true).writeFloat(*gain);
    MemoryOutputStream(destData, true).writeFloat(*density);
    MemoryOutputStream(destData, true).writeFloat(*overall_frequency);
    MemoryOutputStream(destData, true).writeFloat(*randomness);
    MemoryOutputStream(destData, true).writeFloat(*decay_frequency);
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
    overall_frequency->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
    start->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
    decay_frequency->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
    randomness->setValueNotifyingHost(
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

// struct ValueChangeListener : public AudioProcessorParameter::Listener
// {
//   ValueChangeListener(Raindrops &drops) : drops_ref(drops) {}
//   // if value changed, recompute all the parameters
//   void parameterValueChanged(int parameterIndex, float newValue) override
//   {
//     drops_ref.drops_v1->recompute(0.5, newValue, 1.5, 2.0, 25.0, 2.0);
//   }
//   // this one does nothing
//   void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override
//   {
//     return;
//   };

// private:
//   Raindrops &drops_ref;
// };

// class DensityParam : public AudioParameterFloat
// {
// public:
//   DensityParam(const ParameterID &paramID, const String &name,
//                NormalisableRange<float> normalisableRange,
//                float defaultValue)
//       : AudioParameterFloat(paramID, name, normalisableRange, defaultValue){};

//   void valueChanged(float new_value) override
//   {
//     std::cout << "value changed" << std::endl;
//   }
// };