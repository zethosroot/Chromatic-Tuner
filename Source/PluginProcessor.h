/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

static constexpr int YIN_BUFFER_SIZE = 2048;

//==============================================================================
/**
*/
class TunerPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TunerPluginAudioProcessor();
    ~TunerPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    std::atomic<float> m_detectedHz{-1.0f};
    std::atomic<float> m_detectedCents{0.0f};
    std::atomic<int> m_detectedNote{-1};

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TunerPluginAudioProcessor)

    std::array<float, YIN_BUFFER_SIZE> m_ringBuffer{}; // Hold a sample to work with
    int m_ringWritePos = 0; // Indexing
    double m_sampleRate = 44100.0; // Set the sample rate.
    bool m_bufferFull = false;
};
