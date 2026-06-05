/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TunerPluginAudioProcessor::TunerPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

TunerPluginAudioProcessor::~TunerPluginAudioProcessor()
{
}

//==============================================================================
const juce::String TunerPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TunerPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TunerPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TunerPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TunerPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TunerPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TunerPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TunerPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TunerPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void TunerPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TunerPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    m_ringBuffer.fill(0.0f); // Hold a sample to work with
    m_ringWritePos = 0; // Indexing
    m_sampleRate = sampleRate; // Set the sample rate.
    m_bufferFull = false; // Buffer is empty

}

void TunerPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TunerPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


static float detectPitch(const float* buf, int bufSize, double sampleRate) {
    
    int half = bufSize / 2;

    std::vector<float> d(half, 0.0f); // Difference
    std::vector<float> d_norm(half, 0.0f); // Normalization

    for (int tau = 1; tau < half; ++tau)
    {
        for (int i = 0; i < half; ++i)
        {
            float diff = buf[i] - buf[i + tau];
            d[tau] += diff * diff;
        }
    }

    d_norm[0] = 1.0f;

    float runningSum = 0.0f;

    for (int tau = 1; tau < half; ++tau) {
        runningSum += d[tau];
        d_norm[tau] = (runningSum > 0.0f) ? d[tau] * tau / runningSum : 1.0f; // Avoid division by 0
    }

    constexpr float THRESHOLD = 0.12f;

    for (int tau = 2; tau < half; ++tau)
    {
        if (d_norm[tau] < THRESHOLD)
        {
            while (tau + 1 < half && d_norm[tau + 1] < d_norm[tau]) ++tau;

            return (float)(sampleRate / tau);
        }
    }
    return -1.0f;
}

void TunerPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto* data = buffer.getReadPointer(0);
    int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; i++) {
        m_ringBuffer[m_ringWritePos] = data[i];
        m_ringWritePos = (m_ringWritePos + 1) % YIN_BUFFER_SIZE;

        if (m_ringWritePos == 0) m_bufferFull = true;
    }

    if (m_bufferFull) {

        double frequency = -1.0; // Placeholder

        if (frequency >= 50 && frequency <= 1400)
            m_detectedHz.store(frequency, std::memory_order_relaxed);
        else {
            m_detectedHz.store(-1.0f, std::memory_order_relaxed);
        }

        m_bufferFull = false;    

    }

    
}

//==============================================================================
bool TunerPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TunerPluginAudioProcessor::createEditor()
{
    return new TunerPluginAudioProcessorEditor (*this);
}

//==============================================================================
void TunerPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TunerPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TunerPluginAudioProcessor();
}
