/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

static constexpr int YIN_BUFFER_SIZE = 2048;

extern "C" {
    #include "dywapitchtrack.h"
}

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
    std::atomic<double> m_detectedHz{-1.0f};
    std::atomic<double> m_detectedCents{0.0f};
    std::atomic<int> m_detectedNote{-1};

    std::atomic<int> m_tunerMode{ 0 }; // 0 = chromatic, 1 = guitar, 2 = bass
    std::atomic<int> m_language{ 0 }; // 0=EN, 1=DE, 2=HU
    std::atomic<bool> m_sharpPresent{ true }; // For sharp/flat display

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TunerPluginAudioProcessor)

    dywapitchtracker m_pitchTracker;

    std::array<double, YIN_BUFFER_SIZE> m_ringBuffer{}; // Hold a sample to work with
    int m_ringWritePos = 0; // Indexing
    double m_sampleRate = 44100.0; // Set the sample rate temporarily. It will be set properly by DAW.
    bool m_bufferFull = false;

    int m_silenceCounter = 0;
    const int SILENCE_HOLD_FRAMES = 30;
    
    double m_smoothedHz = -1.0;
};
